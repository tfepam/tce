/*
    Copyright 2002-2008 Tampere University of Technology.  All Rights
    Reserved.

    This file is part of TTA-Based Codesign Environment (TCE).

    TCE is free software; you can redistribute it and/or modify it under the
    terms of the GNU General Public License version 2 as published by the Free
    Software Foundation.

    TCE is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
    details.

    You should have received a copy of the GNU General Public License along
    with TCE; if not, write to the Free Software Foundation, Inc., 51 Franklin
    St, Fifth Floor, Boston, MA  02110-1301  USA

    As a special exception, you may use this file as part of a free software
    library without restriction.  Specifically, if other files instantiate
    templates or use macros or inline functions from this file, or you compile
    this file and link it with other files to produce an executable, this
    file does not by itself cause the resulting executable to be covered by
    the GNU General Public License.  This exception does not however
    invalidate any other reasons why the executable file might be covered by
    the GNU General Public License.
*/
/**
 * @file Application.cc
 *
 * Implementation of Application class.
 *
 * Application is a class for generic services that are project-wide
 * applicable to standalone applications or modules. These services include
 * assertion, program exiting, debugging to a log file, catching unexpected
 * exceptions, "control-c" signal handling.
 *
 * @author Atte Oksman 2003 (oksman@cs.tut.fi)
 * @author Pekka Jääskeläinen 2005 (pjaaskel@cs.tut.fi)
 */

#include <string>
#include <iostream>
#include <fstream>
#include <cstddef>
#include <exception>
#include <cstdio>

// for backtrace printing:
#include <signal.h>

// macros to evaluate exit status of pclose()
#include <sys/wait.h>

#include "Environment.hh"
#include "Application.hh"
#include "Exception.hh"
#include "config.h"

using std::fgets; // cstdio
using std::exit;
using std::abort;
using std::atexit;
using std::string;
using std::cerr;
using std::cout;
using std::endl;
using std::ofstream;
using std::ios;
using std::set_unexpected;


// static member variable initializations
bool Application::initialized_ = false;
std::ostream* Application::logStream_ = NULL;
std::map<int, Application::UnixSignalHandler*> Application::signalHandlers_;

int Application::verboseLevel_ = Application::VERBOSE_LEVEL_DEFAULT;

/**
 * Initializes the state data needed by the application services.
 */
void
Application::initialize() {

    // ensure that initialization is done only once per application
    if (initialized_) {
        return;
    }

    // if this is a developer version, we can output debug messages to
    // cerr, in 'distributed version', we'll output the debug messages to
    // a file
    if (!DISTRIBUTED_VERSION) {
        logStream_ = &cerr;
    } else {
        ofstream* fileLog = new ofstream;
        fileLog->open(Environment::errorLogFilePath().c_str(), ios::app);
        if (!fileLog->is_open()) {
            logStream_ = &cerr;
        } else {
            logStream_ = fileLog;
        }
    }

    // set the unexpected exception callback
    set_unexpected(Application::unexpectedExceptionHandler);

    // register finalization function to be called when exit() is called
    // so Application is finalized automatically on program exit
    if (atexit(Application::finalize) != 0) {
        writeToErrorLog(__FILE__, __LINE__, __FUNCTION__,
            "Application initialization failed.");
        abortProgram();
    }

    initialized_ = true;
}

/**
 * Allows writing to the log stream with stream operators.
 *
 * Usage example: logStream() << "Debug output" << i << endl;
 *
 * @return A reference to the log stream.
 */
std::ostream&
Application::logStream() {
    initialize();
    return *logStream_;
}

/**
 * Cleans up the state data used by the application services.
 *
 * This method is called automatically when program is terminated.
 */
void
Application::finalize() {

    if (!initialized_) {
        return;
    }

    if (logStream_ != &cerr && logStream_ != &cout && logStream_ != NULL) {
        logStream_->flush();
        delete logStream_;
        logStream_ = NULL;
    }

    initialized_ = false;
}


/**
 * Records a message into the error log.
 *
 * @param fileName Source file of the code where the error occurred.
 * @param lineNumber Source line where the error occurred.
 * @param functionName Function where the error occurred.
 * @param message The error message.
 */
void
Application::writeToErrorLog(
    const string fileName,
    const int lineNumber,
    const string functionName,
    const string message,
    const int neededVerbosity) {

    if (neededVerbosity > verboseLevel_) {
        return;
    }

    if (!initialized_) {
        initialize();
    }

    *logStream_ << fileName << ":" << lineNumber << ": ";

    if (functionName != UNKNOWN_FUNCTION) {
        *logStream_ <<  "In function \'" << functionName << "\': ";
    }

    *logStream_ << message << endl;
}


/**
 * Exits the program in a normal situation.
 *
 * This method must be used when the program terminates due to nominal
 * conditions without returning from main().
 *
 * @param status Program's status of exit.
 */
void
Application::exitProgram(const int status) {
    exit(status);
}

/**
 * Exit the program in an abnormal situation.
 */
void
Application::abortProgram() {
    abort();
}

/**
 * Default callback for unexpected exceptions.
 */
void
Application::unexpectedExceptionHandler() {
    *logStream_
        << std::endl
        << "Program aborted because of leaked unexpected exception. "
        << std::endl << std::endl <<
        "Information of the last thrown TCE exception: " << std::endl
        << Exception::lastExceptionInfo() << std::endl;
    abortProgram();
}

/**
 * Runs a shell command and captures its output (stdout) in the given vector.
 *
 * Assumes that the executed program does not block and wait for input.
 *
 * @param command Command to execute.
 * @param outputLines Vector to which put the output lines.
 * @param maxOutputLines Maximum lines to capture.
 * @return The return value of the program. -1 if some weird error occured.
 */
int
Application::runShellCommandAndGetOutput(
    const std::string& command,
    std::vector<std::string>& outputLines,
    std::size_t maxOutputLines) {

    char line[MAX_OUTPUT_LINE_LENGTH];

    // flush all streams to avoid: "...the output from a
    // command opened for writing may become intermingled with that of
    // the original process." (man popen)
    fflush(NULL);

    FILE* pipe = popen(command.c_str(), "r");

    while (fgets(line, MAX_OUTPUT_LINE_LENGTH, pipe) == line) {

        if (outputLines.size() < maxOutputLines) {
            outputLines.push_back(string(line));
        }
    }

    int exitStatus = pclose(pipe);

    // see man wait4 for info about macros WIFEXITED and WEXITSTATUS
    if (WIFEXITED(exitStatus)) {
        return WEXITSTATUS(exitStatus);
    }

    return -1;
}

/**
 * Sets a new signal handler for the given signal
 *
 * @param signalNum signal number
 * @param handler The handler to be set
 */
void 
Application::setSignalHandler(int signalNum, UnixSignalHandler& handler) {
    signalHandlers_[signalNum] = &handler;
    
    struct sigaction action;
    action.sa_flags = SA_SIGINFO; 
    action.sa_sigaction = signalRedirector;
 
    sigaction(signalNum, &action, NULL);
}

/**
 * Returns a pointer to the signal's current handler
 * 
 * @return a pointer to the signal's current handler
 * @exception InstanceNotFound if the signal has not been set a custom handler
 */
Application::UnixSignalHandler*
Application::getSignalHandler(int signalNum) {
    std::map<int, UnixSignalHandler*>::iterator it =
        signalHandlers_.find(signalNum);
    if (it != signalHandlers_.end()) {
        return it->second;
    } else {
        throw InstanceNotFound(__FILE__, __LINE__, __FUNCTION__);
    }
}

/**
 * Restores to the signal its original handler
 * 
 * @param signalNum signal number
 */
void 
Application::restoreSignalHandler(int signalNum) {
    signal(signalNum, SIG_DFL);
    signalHandlers_.erase(signalNum);
}

/**
 * Redirects the signal received to the current signal handler
 *
 * @param data Data from the signal.
 * @param info signal information struct
 * @param context signal context
 */
void
Application::signalRedirector(int data, siginfo_t *info, void *context) {
    if (context) {}
    
    UnixSignalHandler* handler = getSignalHandler(info->si_signo);
    assert(handler != NULL);
    handler->execute(data, info);
}

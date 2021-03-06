/*
    Copyright (c) 2002-2009 Tampere University.

    This file is part of TTA-Based Codesign Environment (TCE).

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
 */
/**
 * @file ProximOpenProgramCmd.cc
 *
 * Implementation of ProximOpenProgramCmd class.
 *
 * @author Veli-Pekka Jääskeläinen 2005 (vjaaskel-no.spam-cs.tut.fi)
 * @note rating: red
 */

#include "ProximOpenProgramCmd.hh"
#include "WxConversion.hh"
#include "ProximConstants.hh"
#include "Proxim.hh"
#include "ProximSimulationThread.hh"

#if wxCHECK_VERSION(3, 0, 0)
    #define wxOPEN wxFD_OPEN
#endif

/**
 * The Constructor.
 */
ProximOpenProgramCmd::ProximOpenProgramCmd():
    GUICommand(ProximConstants::COMMAND_NAME_OPEN_PROGRAM, NULL) {
}


/**
 * The Destructor.
 */
ProximOpenProgramCmd::~ProximOpenProgramCmd() {
}


/**
 * Executes the command.
 */
bool
ProximOpenProgramCmd::Do() {
    assert(parentWindow() != NULL);

    wxString wildcard = _T("TPEF Program Files (*.tpf, *.tpef)|*.tpf;*.tpef");
    wildcard.Append(_T("|All files|*"));
    wxFileDialog dialog(
	parentWindow(), _T("Choose a file."), _T(""), _T(""),
	wildcard, wxOPEN);

    if (dialog.ShowModal() == wxID_CANCEL) {
	return false;
    }

    std::string command;
    std::string file = WxConversion::toString(dialog.GetPath());

    command = ProximConstants::SCL_LOAD_PROGRAM + " \"" + file + "\"";
    wxGetApp().simulation()->lineReader().input(command);

    return true;
}


/**
 * Returns full path to the command icon file.
 *
 * @return Full path to the command icon file.
 */
std::string
ProximOpenProgramCmd::icon() const {
    return "open_program.png";
}


/**
 * Returns ID of this command.
 */
int
ProximOpenProgramCmd::id() const {
    return ProximConstants::COMMAND_OPEN_PROGRAM;
}


/**
 * Creates and returns a new instance of this command.
 *
 * @return Newly created instance of this command.
 */
ProximOpenProgramCmd*
ProximOpenProgramCmd::create() const {
    return new ProximOpenProgramCmd();
}


/**
 * Returns true if the command is enabled, false otherwise.
 *
 * @return Always true.
 */
bool
ProximOpenProgramCmd::isEnabled() {
    return true;
}


/**
 * Returns shortened name of the command for toolbar button text.
 *
 * @return Short version of the command name.
 */
std::string
ProximOpenProgramCmd::shortName() const {
    return "Program";
}

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
 * @file ModifyFUPortCmd.cc
 *
 * Definition of ModifyFUPortCmd class.
 *
 * @author Veli-Pekka Jääskeläinen 2004 (vjaaskel-no.spam-cs.tut.fi)
 * @note rating: red
 */

#include "Application.hh"
#include "ModifyFUPortCmd.hh"
#include "ModelConstants.hh"
#include "EditPart.hh"
#include "FUPortDialog.hh"
#include "FUPort.hh"

using namespace TTAMachine;

/**
 * The Constructor.
 *
 * @param editPart Port to modify.
 */
ModifyFUPortCmd::ModifyFUPortCmd(EditPart* editPart):
    ComponentCommand(),
    editPart_(editPart) {
}


/**
 * The Destructor.
 */
ModifyFUPortCmd::~ModifyFUPortCmd() {
}


/**
 * Executes the command.
 *
 * @return true, if the command was succesfully executed, false otherwise.
 */
bool
ModifyFUPortCmd::Do() {

    FUPort* port = dynamic_cast<FUPort*>(editPart_->model());

    assert (port != NULL);
    assert (parentWindow() != NULL);

    FUPortDialog dialog(parentWindow(), port);

    if (dialog.ShowModal() == wxID_OK) {
	// port was modified
	return true;
    } else {
	return false;
    }
}

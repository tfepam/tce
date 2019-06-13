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
 * @file ResultDialog.cc
 *
 * Definition of ResultDialog class.
 *
 * @author Jussi Nyk�nen 2004 (nykanen-no.spam-cs.tut.fi)
 * @note rating: red
 */

#include <boost/format.hpp>

#include "ResultDialog.hh"
#include "OSEdTextGenerator.hh"
#include "GUITextGenerator.hh"
#include "WxConversion.hh"
#include "WidgetTools.hh"
#include "DialogPosition.hh"
#include "ErrorDialog.hh"
#include "CommandThread.hh"
#include "FileSystem.hh"
#include "OSEd.hh"
#include "OSEdOptions.hh"

using boost::format;
using std::string;
using std::vector;

BEGIN_EVENT_TABLE(ResultDialog, wxDialog)
    EVT_BUTTON(ID_BUTTON_OPEN, ResultDialog::onOpen)
END_EVENT_TABLE()

/**
 * Constructor.
 *
 * @param parent Parent window.
 */
ResultDialog::ResultDialog(
    wxWindow* parent, 
    std::vector<std::string> output,
    const std::string& title,
    const std::string& module) :
    wxDialog(parent, -1, _T(""), 
             DialogPosition::getPosition(DialogPosition::DIALOG_RESULT),
             wxDefaultSize, wxRESIZE_BORDER), 
    output_(output), module_(module) {

    createContents(this, true, true);
    result_ = dynamic_cast<wxTextCtrl*>(FindWindow(ID_RESULT));
    result_->SetEditable(false);

    FindWindow(wxID_OK)->SetFocus();
	SetTitle(WxConversion::toWxString(title));
    if (module_ == "") {
        FindWindow(ID_BUTTON_OPEN)->Disable();
    }
    setTexts();
}

/**
 * Destructor.
 */
ResultDialog::~ResultDialog() {
    int x, y;
    GetPosition(&x, &y);
    wxPoint point(x, y);
    DialogPosition::setPosition(DialogPosition::DIALOG_RESULT, point);
}

/**
 * Set texts to widgets.
 */
void
ResultDialog::setTexts() {
	
    GUITextGenerator& guiText = *GUITextGenerator::instance();
    OSEdTextGenerator& osedText = OSEdTextGenerator::instance();

    // buttons
    WidgetTools::setLabel(&guiText, FindWindow(wxID_OK),
                          GUITextGenerator::TXT_BUTTON_OK);
    
    WidgetTools::setLabel(&osedText, FindWindow(ID_BUTTON_OPEN),
                          OSEdTextGenerator::TXT_BUTTON_OPEN);
}

/**
 * Transfer data to window.
 *
 * @return True if transfer is successful.
 */
bool
ResultDialog::TransferDataToWindow() {
    for (size_t i = 0; i < output_.size(); i++) {
        result_->AppendText(WxConversion::toWxString(output_[i]));
    }
    return wxWindow::TransferDataToWindow();
}

/**
 * Handles the event when Open button is pushed.
 */
void
ResultDialog::onOpen(wxCommandEvent&) {
    
    OSEdTextGenerator& texts = OSEdTextGenerator::instance();
    OSEdOptions* options = wxGetApp().options();
    string editor = options->editor();
    if (editor == "") {
        format fmt = texts.text(OSEdTextGenerator::TXT_ERROR_NO_EDITOR);
        ErrorDialog error(this, WxConversion::toWxString(fmt.str()));
        error.ShowModal();
    } else {
        if (FileSystem::fileExists(editor)) {
            string cmd = editor + " " + module_;
            CommandThread* thread = new CommandThread(cmd);
            thread->Create();
            thread->Run();
        } else {
            format fmt = texts.text(OSEdTextGenerator::TXT_ERROR_OPEN);
            fmt % editor;
            ErrorDialog dialog(this, WxConversion::toWxString(fmt.str()));
            dialog.ShowModal();
        }
    }
}

/**
 * Creates the contents of the dialog.
 *
 * NOTE! This function was generated by wxDesigner.
 *
 * @param parent Parent window.
 * @param call_fit If true fits the contents inside the dialog.
 * @param set_sizer If true, sets the main sizer as contents of dialog.
 * @return The created sizer.
 */
wxSizer*
ResultDialog::createContents(
    wxWindow *parent, 
    bool call_fit, 
    bool set_sizer) {

    wxBoxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxTextCtrl *item1 = new wxTextCtrl( parent, ID_RESULT, wxT(""), wxDefaultPosition, wxSize(400,100), wxTE_MULTILINE );
    item0->Add( item1, 1, wxGROW|wxALIGN_CENTER|wxALL, 5 );

    wxBoxSizer *item2 = new wxBoxSizer( wxHORIZONTAL );

    wxButton *item3 = new wxButton( parent, ID_BUTTON_OPEN, wxT("Open"), wxDefaultPosition, wxDefaultSize, 0 );
    item2->Add( item3, 0, wxALIGN_CENTER|wxALL, 5 );

    wxButton *item4 = new wxButton( parent, wxID_OK, wxT("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    item2->Add( item4, 0, wxALIGN_CENTER|wxALL, 5 );

    item0->Add( item2, 0, wxALIGN_CENTER|wxALL, 5 );

    if (set_sizer)
    {
        parent->SetSizer( item0 );
        if (call_fit)
            item0->SetSizeHints( parent );
    }
    
    return item0;
}

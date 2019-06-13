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
 * @file EditPolicy.hh
 *
 * Declaration of EditPolicy class.
 *
 * @author Ari Metsähalme 2003 (ari.metsahalme-no.spam-tut.fi)
 * @note rating: yellow
 * @note reviewed Jul 20 2004 by vpj, jn, am
 */

#ifndef TTA_EDIT_POLICY_HH
#define TTA_EDIT_POLICY_HH

class EditPart;
class Request;
class ComponentCommand;
 
/**
 * Determines how an EditPart acts when a Request is performed on it.
 *
 * Converts a given Request to a Command if the EditPolicy supports
 * the Request.
 */
class EditPolicy {
public:
    EditPolicy();
    virtual ~EditPolicy();

    EditPart* host() const;
    void setHost(EditPart* host);

    /**
     * Returns the Command corresponding to the type of the Request.
     *
     * @param request Request to be handled.
     * @return NULL if the Request cannot be handled.
     */
    virtual ComponentCommand* getCommand(Request* request) = 0;

    /**
     * Tells whether this EditPolicy is able to handle a certain type
     * of Request.
     *
     * @param request Request to be asked if it can be handled.
     * @return True if the Request can be handled, false otherwise.
     */
    virtual bool canHandle(Request* request) const = 0;

protected:
    /// Host EditPart of this EditPolicy.
    EditPart* host_;

private:
    /// Assignment not allowed.
    EditPolicy& operator=(EditPolicy& old);
    /// Copying not allowed.
    EditPolicy(EditPolicy& old);
};

#include "EditPolicy.icc"

#endif

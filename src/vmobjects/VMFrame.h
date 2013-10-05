#pragma once
#ifndef VMFRAME_H_
#define VMFRAME_H_

/*
 *
 *
 Copyright (c) 2007 Michael Haupt, Tobias Pape, Arne Bergmann
 Software Architecture Group, Hasso Plattner Institute, Potsdam, Germany
 http://www.hpi.uni-potsdam.de/swa/

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#include "VMArray.h"
class VMMethod;
class VMObject;
class VMInteger;

class VMFrame: public VMArray {
public:
    static pVMFrame EmergencyFrameFrom(pVMFrame from, long extraLength);

    VMFrame(long size, long nof = 0);

    virtual inline pVMFrame GetPreviousFrame() const;
    virtual inline void SetPreviousFrame(pVMFrame);
    virtual inline void ClearPreviousFrame();
    virtual bool HasPreviousFrame() const;
    virtual inline bool IsBootstrapFrame() const;
    virtual inline pVMFrame GetContext() const;
    virtual inline void SetContext(pVMFrame);
    virtual bool HasContext() const;
    virtual pVMFrame GetContextLevel(long);
    virtual pVMFrame GetOuterContext();
    virtual pVMMethod GetMethod() const;
    virtual void SetMethod(pVMMethod);
    virtual pVMObject Pop();
    virtual void Push(pVMObject);
    virtual void ResetStackPointer();
    virtual long GetBytecodeIndex() const;
    virtual void SetBytecodeIndex(long);
    virtual pVMObject GetStackElement(long) const;
    virtual void SetStackElement(long, pVMObject);
    virtual pVMObject GetLocal(long, long);
    virtual void SetLocal(long, long, pVMObject);
    virtual pVMObject GetArgument(long, long);
    virtual void SetArgument(long, long, pVMObject);
    virtual void PrintStackTrace() const;
    virtual long ArgumentStackIndex(long index) const;
    virtual void CopyArgumentsFrom(pVMFrame frame);

    virtual void MarkReferences();
    virtual void PrintStack() const;
    virtual inline pVMInteger GetStackPointer() const;
    virtual long RemainingStackSize() const;
private:
    pVMFrame previousFrame;
    pVMFrame context;
    pVMMethod method;
    pVMInteger stackPointer;
    pVMInteger bytecodeIndex;
    pVMInteger localOffset;

    static const long VMFrameNumberOfFields;
};

bool VMFrame::IsBootstrapFrame() const {
    return !HasPreviousFrame();
}

pVMFrame VMFrame::GetContext() const {
    return this->context;
}

void VMFrame::SetContext(pVMFrame frm) {
    this->context = frm;
}

pVMInteger VMFrame::GetStackPointer() const {
    return stackPointer;
}

pVMFrame VMFrame::GetPreviousFrame() const {
    return this->previousFrame;
}

void VMFrame::SetPreviousFrame(pVMFrame frm) {
    this->previousFrame = frm;
}

void VMFrame::ClearPreviousFrame() {
    this->previousFrame = static_cast<pVMFrame>(nilObject);
}

#endif

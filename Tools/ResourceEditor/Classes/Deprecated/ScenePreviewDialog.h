/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __SCENE_PREVIEW_DIALOG_H__
#define __SCENE_PREVIEW_DIALOG_H__

#include "DAVAEngine.h"
#include "Deprecated/ExtendedDialog.h"

using namespace DAVA;


class ScenePreviewControl;
class ScenePreviewDialog: public ExtendedDialog
{
public:
    ScenePreviewDialog();
    virtual ~ScenePreviewDialog();
    
    void Show(const FilePath &scenePathname);
    virtual void Close();
    
protected:

    virtual const Rect GetDialogRect() const;
    virtual void UpdateSize();
    
    void OnClose(BaseObject *, void *, void *);

    ScenePreviewControl* preview = nullptr;
    UIStaticText* errorMessage = nullptr;
    UIControl* clickableBackgound = nullptr;
};



#endif // __SCENE_PREVIEW_DIALOG_H__
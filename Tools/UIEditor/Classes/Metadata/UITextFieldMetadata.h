/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/
#ifndef UITEXTFIELDMETADATA_H
#define UITEXTFIELDMETADATA_H

#include "UITextControlMetadata.h"
#include "UI/UITextField.h"

namespace DAVA {

// Metadata class for DAVA text-field controls.
class UITextFieldMetadata : public UITextControlMetadata
{
    Q_OBJECT

    // Text field text Properties
    Q_PROPERTY(QString Text READ GetText WRITE SetText);
    Q_PROPERTY(QColor TextColor READ GetTextColor WRITE SetTextColor);

public:
    UITextFieldMetadata(QObject* parent = 0);

protected:
	virtual QString GetUIControlClassName() { return "UITextField"; };
	
    // Initialization.
    void InitializeControl(const String& controlName, const Vector2& position);

    // Helper to access active UI Text Field.
    UITextField* GetActiveUITextField() const;
     // Getters/setters.
    QString GetText() const;
    void SetText(const QString& text);
    
    virtual Font * GetFont();
    virtual void SetFont(Font* font);

    virtual float GetFontSize() const;
    virtual void SetFontSize(float fontSize);

    virtual QColor GetFontColor() const;
    virtual void SetFontColor(const QColor& value);

    QColor GetTextColor() const;
    void SetTextColor(const QColor& color);
	
	virtual float GetShadowOffsetX() const;
	virtual void SetShadowOffsetX(float offset);
	
	virtual float GetShadowOffsetY() const;
	virtual void SetShadowOffsetY(float offset);
	
	virtual QColor GetShadowColor() const;
	virtual void SetShadowColor(const QColor& value);

	virtual int GetTextAlign();
    virtual void SetTextAlign(int align);
};

};


#endif // UITEXTFIELDMETADATA_H

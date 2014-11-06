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



#include "AndroidLayer.h"
#include "UI/UITextFieldAndroid.h"
#include "Base/BaseTypes.h"
#include "Utils/UTF8Utils.h"

extern "C"
{
	void Java_com_dava_framework_JNITextField_TextFieldShouldReturn(JNIEnv* env, jobject classthis, uint32_t id)
	{
		DAVA::UITextFieldAndroid::TextFieldShouldReturn(id);
	}

	jstring Java_com_dava_framework_JNITextField_TextFieldKeyPressed(JNIEnv* env, jobject classthis, uint32_t id, int replacementLocation, int replacementLength, jbyteArray replacementString)
	{
		DAVA::WideString string;

		jbyte* bufferPtr = env->GetByteArrayElements(replacementString, NULL);
		jsize lengthOfArray = env->GetArrayLength(replacementString);

		DAVA::UTF8Utils::EncodeToWideString((uint8_t*)bufferPtr, lengthOfArray, string);

		env->ReleaseByteArrayElements(replacementString, bufferPtr, 0);

		bool res = DAVA::UITextFieldAndroid::TextFieldKeyPressed(id, replacementLocation, replacementLength, string);
		DAVA::String returnStr = res ? DAVA::UTF8Utils::EncodeToUTF8(string) : "";

		return env->NewStringUTF(returnStr.c_str());
	}

	void Java_com_dava_framework_JNITextField_TextFieldKeyboardShown(JNIEnv* env, jobject classthis, uint32_t id, int x, int y, int dx, int dy)
	{
	    // Recalculate to virtual coordinates.
	    DAVA::Vector2 keyboardOrigin(x, y);
	    keyboardOrigin *= DAVA::UIControlSystem::Instance()->GetScaleFactor();
	    keyboardOrigin += DAVA::UIControlSystem::Instance()->GetInputOffset();

	    DAVA::Vector2 keyboardSize(dx, dy);
	    keyboardSize *= DAVA::UIControlSystem::Instance()->GetScaleFactor();
	    keyboardSize += DAVA::UIControlSystem::Instance()->GetInputOffset();

	    DAVA::UITextFieldAndroid::TextFieldKeyboardShown(id, DAVA::Rect(keyboardOrigin, keyboardSize));
	}

	void Java_com_dava_framework_JNITextField_TextFieldKeyboardHidden(JNIEnv* env, jobject classthis, uint32_t id)
	{
	    DAVA::UITextFieldAndroid::TextFieldKeyboardHidden(id);
	}

    void Java_com_dava_framework_JNITextField_TextFieldFocusChanged(JNIEnv* env, jobject classthis, uint32_t id, bool hasFocus)
    {
        DAVA::UITextFieldAndroid::TextFieldFocusChanged(id, hasFocus);
    }

};

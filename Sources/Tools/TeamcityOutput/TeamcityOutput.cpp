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



#include "TeamcityOutput.h"
#include "Utils/Utils.h"

#include <iostream>


namespace DAVA
{
    
void TeamcityOutput::Output(Logger::eLogLevel ll, const char8 *text)
{
    String outStr = NormalizeString(text);
	String output;
    String status;
    
    switch (ll)
    {
    case Logger::LEVEL_ERROR:
		status = "ERROR";
        output = "##teamcity[buildProblem description=\'ERROR: " + outStr + "\']";
        PlatformOutput(output);
        break;

    case Logger::LEVEL_WARNING:
		status = "WARNING";
        break;
            
    default:
        status = "NORMAL";
        break;
    }

    output = "##teamcity[message text=\'" + outStr + "\' errorDetails=\'\' status=\'" + status + "\']\n";
    PlatformOutput(output);
}

void TeamcityOutput::Output(Logger::eLogLevel ll, const char16 *text)
{
    WideString wstr = text;
    Output(ll, WStringToString(wstr).c_str());
}

String TeamcityOutput::NormalizeString(const char8 *text) const
{
    String str = text;
    
    StringReplace(str, "|", "||");

    StringReplace(str, "'", "|'");
    StringReplace(str, "\n", "|n");
    StringReplace(str, "\r", "|r");

//    StringReplace(str, "\u0085", "|x");
//     StringReplace(str, "\u2028", "|l");
//     StringReplace(str, "\u2029", "|p");

    StringReplace(str, "[", "|[");
    StringReplace(str, "]", "|]");
    
    return str;
}

void TeamcityOutput::PlatformOutput(const String &text) const
{
#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_ANDROID__)
    std::cout << text << std::endl;
#else
    NSLog(@"%s", text.c_str());
#endif // defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_ANDROID__)
}
    
}; // end of namespace DAVA


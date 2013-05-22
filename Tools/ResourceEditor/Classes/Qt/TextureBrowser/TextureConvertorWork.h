/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#ifndef __TEXTURE_CONVERTOR_WORK_H__
#define __TEXTURE_CONVERTOR_WORK_H__

#include "DAVAEngine.h"
#include "Render/TextureDescriptor.h"

struct JobItem
{
	enum JobItemType
	{
		JobOriginal,
		JobPVR,
		JobDXT
	};

	JobItem()
		: forceConvert(false)
		, type(JobOriginal)
		, descriptor(NULL)
	{ }

	bool forceConvert;
	JobItemType type;
	const DAVA::TextureDescriptor *descriptor;

	// grant thread safe access to descriptor
	DAVA::TextureDescriptor descriptorCopy;
};

class JobStack
{
public:
	JobStack();
	~JobStack();

	void push(const JobItem &item);
	JobItem* pop();
	int size();

private:
	struct JobItemWrapper : public JobItem
	{
		JobItemWrapper(const JobItem &item);

		JobItemWrapper *next;
		JobItemWrapper *prev;
	};

	JobItemWrapper *head;

	int itemsCount;
};

#endif // __TEXTURE_CONVERTOR_WORK_H__

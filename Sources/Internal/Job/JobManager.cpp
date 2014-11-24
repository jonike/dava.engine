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

#include "Job/JobManager.h"
#include "Debug/DVAssert.h"
#include "Base/ScopedPtr.h"
#include "Platform/Thread.h"
#include "Thread/LockGuard.h"
#include "Platform/DeviceInfo.h"

namespace DAVA
{

JobManager::JobManager()
: workerDoneSem(0)
{
	uint32 cpuCoresCount = DeviceInfo::GetCpuCount();
	workerThreads.reserve(cpuCoresCount);

	for (uint32 i = 0; i < cpuCoresCount; ++i)
	{
		WorkerThread * thread = new WorkerThread(&workerQueue, &workerDoneSem);
		workerThreads.push_back(thread);
	}
}

JobManager::~JobManager()
{
	for (uint32 i = 0; i < workerThreads.size(); ++i)
	{
		SafeDelete(workerThreads[i]);
	}

	workerThreads.clear();
}

void JobManager::Update()
{
	LockGuard<Mutex> guard(mainQueueMutex);

	if(!mainJobs.empty())
	{
		// extract all jobs from queue
		while(!mainJobs.empty())
		{
			curMainJob = mainJobs.front();
			mainJobs.pop_front();

			if(curMainJob.type == JOB_MAINBG)
			{
				// TODO:
				// need implementation
				// ...

				DVASSERT(false);
			}

			if(curMainJob.invokerThreadId != 0 && curMainJob.fn != NULL)
			{
				// unlock queue mutex until function execution finished
				mainQueueMutex.Unlock();
				curMainJob.fn();
				mainQueueMutex.Lock();
			}

			curMainJob = MainJob();
		}

		{
			// signal that jobs are finished
			LockGuard<Mutex> cvguard(mainCVMutex);
			Thread::Broadcast(&mainCV);
		}
	}
}

uint32 JobManager::GetWorkersCount() const
{
	return workerThreads.size();
}

void JobManager::CreateMainJob(const Function<void()>& fn, eMainJobType mainJobType)
{
	// if we are already in main thread and requested job shouldn't executed lazy
	// perform that job immediately
    if(Thread::IsMainThread() && mainJobType != JOB_MAINLAZY)
    {
        fn();
    }
    else
    {
		// push requested job into queue
        MainJob job;
        job.fn = fn;
        job.invokerThreadId = Thread::GetCurrentId();
		job.type = mainJobType;

		{
			LockGuard<Mutex> guard(mainQueueMutex);
			mainJobs.push_back(job);
		}
    }
}

void JobManager::WaitMainJobs(Thread::Id invokerThreadId /* = 0 */)
{
 	if(Thread::IsMainThread())
 	{
		// if wait was invoked from main-thread we should immediately
		// execute all lazy main-thread jobs
 		Update();
 	}
 	else
 	{
		// If main thread is locked by WaitWorkerJobs this instruction will unlock
		// main thread, allowing it to perform all scheduled main-thread jobs
		workerDoneSem.Post();

		// Now check if there are some jobs in the queue and wait for them
		LockGuard<Mutex> guard(mainCVMutex);
		while (HasMainJobs(invokerThreadId))
		{
			Thread::Wait(&mainCV, &mainCVMutex);
		}
	}
}

bool JobManager::HasMainJobs(Thread::Id invokerThreadId /* = 0 */) const
{
    bool ret = false;

	// tread id = 0 as current thread id, so we should get it
    if(0 == invokerThreadId)
    {
        invokerThreadId = Thread::GetCurrentId();
    }

	{
		LockGuard<Mutex> guard(mainQueueMutex);
		if(curMainJob.invokerThreadId == invokerThreadId)
		{
			ret = true;
		}
		else
		{
			Deque<MainJob>::const_iterator i = mainJobs.begin();
			Deque<MainJob>::const_iterator end = mainJobs.end();
			for(; i != end; ++i)
			{
				if(i->invokerThreadId == invokerThreadId)
				{
					ret = true;
					break;
				}
			}
		}
	}

    return ret;
}

void JobManager::CreateWorkerJob(const Function<void()>& fn)
{
	workerQueue.Push(fn);
	workerQueue.Signal();
}

void JobManager::WaitWorkerJobs()
{
	while(HasWorkerJobs())
	{
		if(Thread::IsMainThread())
		{
			// We want to be able to wait worker jobs, but at the same time
			// allow any worker job execute main job. Potentially this will cause
			// dead lock, but there is a simple solution:
			// 
			// Every time, worker job is trying to execute WaitMainJobs it will 
			// post workerDoneSem semaphore, that will give a chance to execute main jobs
			// in the following Update() call
			//
			Update();
		}

		workerDoneSem.Wait();
	}
}

bool JobManager::HasWorkerJobs() const
{
	return !workerQueue.IsEmpty();
}

}

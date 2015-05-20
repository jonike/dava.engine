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

#ifndef __DAVAENGINE_THREADLOCALPTR_H__
#define __DAVAENGINE_THREADLOCALPTR_H__

#include <type_traits>
#include <cassert>

#include "Base/BaseTypes.h"

namespace DAVA
{

/*
    template class ThreadLocalPtr - implementation of cross-platform thread local storage (TLS). Each instance of ThreadLocalPtr
    represents a pointer to an object of type T. Each thread has a distinct value.

    To obtain value for the current thread one can use Get() method, or -> and * pointer dereference operators. Initially pointer
    has nullptr value for each thread, to set value for the current thread one can use Reset() method.

    ThreadLocalPtr's interface is similar to interface of boost::thread_specific_ptr.

    C++ 11 supports thread_local keyword which does the same thing but not all compilers support it.
    Also, compiler specific stuff (__declspec(thread), __thread) does not work well between platforms.

    Restrictions:
        variables of type ThreadLocal can have only static storage duration (global or local static, and static data member)
        if you declare ThreadLocal as automatic object it's your own problems, so don't cry: Houston, we've got a problem

    TODO:
        integrate ThreadLocalPtr into DAVA::Thread to support automatic cleanup on thread exit. For now user is responsible for
        calling ThreadLocalPtr::Reset() to delete pointer
*/
template<typename T>
class ThreadLocalPtr final
{
#if defined(__DAVAENGINE_WIN32__)
    using KeyType = DWORD;
#elif defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
    using KeyType = pthread_key_t;
#else
#   error "ThreadLocalPtr: platform is unknown"
#endif

public:
    ThreadLocalPtr() CC_NOEXCEPT;
    ThreadLocalPtr(void (*deleter_)(T*)) CC_NOEXCEPT;
    ~ThreadLocalPtr() CC_NOEXCEPT;

    T* Get() const CC_NOEXCEPT;
    T* operator -> () const CC_NOEXCEPT;
    T& operator * () const CC_NOEXCEPT;

    // Get current pointer and set nullptr without deleting
    T* Release() CC_NOEXCEPT;
    // Set new pointer, delete previous pointer if it is not same with new pointer
    void Reset(T* newValue = nullptr) CC_NOEXCEPT;

    // Method to test whether thread local storage has been successfully created by system
    bool IsCreated() const CC_NOEXCEPT;

private:
    ThreadLocalPtr(const ThreadLocalPtr&) = delete;
    ThreadLocalPtr& operator = (const ThreadLocalPtr&) = delete;

    // Platform spefific methods
    void CreateTlsKey() CC_NOEXCEPT;
    void DeleteTlsKey() const CC_NOEXCEPT;
    void SetTlsValue(void* rawValue) const CC_NOEXCEPT;
    void* GetTlsValue() const CC_NOEXCEPT;

    static void DefaultDeleter(T* ptr) CC_NOEXCEPT;

private:
    KeyType key;
    bool isCreated = false;
    void(*deleter)(T*);
};

//////////////////////////////////////////////////////////////////////////

template<typename T>
inline ThreadLocalPtr<T>::ThreadLocalPtr() CC_NOEXCEPT
    : deleter(&DefaultDeleter)
{
    CreateTlsKey();
}

template<typename T>
inline ThreadLocalPtr<T>::ThreadLocalPtr(void(*deleter_)(T*)) CC_NOEXCEPT
    : deleter(deleter_)
{
    CreateTlsKey();
}

template<typename T>
inline ThreadLocalPtr<T>::~ThreadLocalPtr() CC_NOEXCEPT
{
    DeleteTlsKey();
}

template<typename T>
inline T* ThreadLocalPtr<T>::Get() const CC_NOEXCEPT
{
    return static_cast<T*>(GetTlsValue());
}

template<typename T>
inline T* ThreadLocalPtr<T>::operator -> () const CC_NOEXCEPT
{
    return Get();
}

template<typename T>
inline T& ThreadLocalPtr<T>::operator * () const CC_NOEXCEPT
{
    return *Get();
}

template<typename T>
inline T* ThreadLocalPtr<T>::Release() CC_NOEXCEPT
{
    T* ptr = Get();
    SetTlsValue(nullptr);
    return ptr;
}

template<typename T>
inline void ThreadLocalPtr<T>::Reset(T* newValue) CC_NOEXCEPT
{
    T* curValue = Get();
    if (curValue != newValue)
    {
        deleter(curValue);
        SetTlsValue(newValue);
    }
}

template<typename T>
inline bool ThreadLocalPtr<T>::IsCreated() const CC_NOEXCEPT
{
    return isCreated;
}

template<typename T>
void ThreadLocalPtr<T>::DefaultDeleter(T* ptr) CC_NOEXCEPT
{
    delete ptr;
}

// Win32 implementation
#if defined(__DAVAENGINE_WIN32__)

template<typename T>
inline void ThreadLocalPtr<T>::CreateTlsKey() CC_NOEXCEPT
{
    key = TlsAlloc();
    isCreated = (key != TLS_OUT_OF_INDEXES);
    assert(isCreated);
}

template<typename T>
inline void ThreadLocalPtr<T>::DeleteTlsKey() const CC_NOEXCEPT
{
    TlsFree(key);
}

template<typename T>
inline void ThreadLocalPtr<T>::SetTlsValue(void* rawValue) const CC_NOEXCEPT
{
    TlsSetValue(key, rawValue);
}

template<typename T>
inline void* ThreadLocalPtr<T>::GetTlsValue() const CC_NOEXCEPT
{
    return TlsGetValue(key);
}

// POSIX implementation
#elif defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)

template<typename T>
inline void ThreadLocalPtr<T>::CreateTlsKey() CC_NOEXCEPT
{
    isCreated = (0 == pthread_key_create(&key, nullptr));
    assert(isCreated);
}

template<typename T>
inline void ThreadLocalPtr<T>::DeleteTlsKey() const CC_NOEXCEPT
{
    pthread_key_delete(key);
}

template<typename T>
inline void ThreadLocalPtr<T>::SetTlsValue(void* rawValue) const CC_NOEXCEPT
{
    pthread_setspecific(key, rawValue);
}

template<typename T>
inline void* ThreadLocalPtr<T>::GetTlsValue() const CC_NOEXCEPT
{
    return pthread_getspecific(key);
}

#endif

}   // namespace DAVA

#endif  // __DAVAENGINE_THREADLOCALPTR_H__


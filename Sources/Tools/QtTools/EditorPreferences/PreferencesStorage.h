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


#pragma once

#include "FileSystem/KeyedArchive.h"
#include "FileSystem/FilePath.h"
#include "Base/StaticSingleton.h"
#include "Functional/Signal.h"

namespace DAVA
{
class InspBase;
};

class PreferencesStorage : public DAVA::StaticSingleton<PreferencesStorage>
{
public:
    using DefaultValuesList = DAVA::Map<DAVA::FastName, DAVA::VariantType>;
    using RegisteredIntrospection = DAVA::Vector<const DAVA::InspInfo*>;

    PreferencesStorage();

    void RegisterType(const DAVA::InspInfo* inspInfo, const DefaultValuesList& defaultValues = DefaultValuesList());
    template <typename T>
    void RegisterPreferences(T* obj);
    template <typename T>
    void UnregisterPreferences(T* obj);

    void SetupStoragePath(const DAVA::FilePath& localStorage);

    //Getter and setter to use preferences manually without introspections. Desired for namespaces and local functions;
    void SaveValueByKey(const DAVA::FastName& key, const DAVA::VariantType& value);
    DAVA::VariantType LoadValueByKey(const DAVA::FastName& key);

    const DAVA::InspInfo* GetInspInfo(const DAVA::FastName& className) const;

    void SetNewValueToAllRegisteredObjects(const DAVA::InspMember* member, const DAVA::VariantType& value);
    DAVA::VariantType GetPreferencesValue(const DAVA::InspMember* member) const;

    const RegisteredIntrospection& GetRegisteredInsp() const;

    struct PreferencesStorageSaver
    {
        PreferencesStorageSaver();
        ~PreferencesStorageSaver();
    };

    DAVA::Signal<const DAVA::InspMember*, const DAVA::VariantType&> ValueChanged;

private:
    void RegisterPreferences(void* realObj, DAVA::InspBase* inspBase);
    void UnregisterPreferences(void* realObj, const DAVA::InspBase* inspBase);

    static DAVA::String GenerateKey(const DAVA::InspInfo* inspInfo);

    DAVA::FilePath localStorage;
    DAVA::ScopedPtr<DAVA::KeyedArchive> editorPreferences;

    RegisteredIntrospection registeredInsp;
    DAVA::Map<const DAVA::InspInfo*, DefaultValuesList> defaultValues;
    DAVA::Map<const DAVA::InspInfo*, DAVA::Set<void*>> registeredObjects;
    DAVA::KeyedArchive* preferencesArchive = nullptr;
    DAVA::KeyedArchive* unnamedPreferencesArchive = nullptr;
    const DAVA::String unnamedPreferencesKey;
    const DAVA::String preferencesKey;
};

template <typename T>
void PreferencesStorage::RegisterPreferences(T* realObject)
{
    static_assert(std::is_base_of<DAVA::InspBase, T>::value, "type T must be derived from InspBase");
    Instance()->RegisterPreferences(realObject, static_cast<DAVA::InspBase*>(realObject));
}

template <typename T>
void PreferencesStorage::UnregisterPreferences(T* realObject)
{
    static_assert(std::is_base_of<DAVA::InspBase, T>::value, "type T must be derived from InspBase");
    Instance()->UnregisterPreferences(realObject, static_cast<DAVA::InspBase*>(realObject));
}

#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/DataProcessing/DataContext.h>

#include <Reflection/Reflection.h>

class SlotSupportModule : public DAVA::TArc::ClientModule
{
protected:
    void OnContextCreated(DAVA::TArc::DataContext* context) override;
    void OnContextDeleted(DAVA::TArc::DataContext* context) override;

    void OnInterfaceRegistered(const DAVA::Type* interfaceType) override;
    void OnBeforeInterfaceUnregistered(const DAVA::Type* interfaceType) override;

    void PostInit() override;

    DAVA_VIRTUAL_REFLECTION(SlotSupportModule, DAVA::TArc::ClientModule);
};

#include "Engine/Private/Win32/DeviceManagerImplWin32.h"

#if defined(__DAVAENGINE_QT__)

#include "Engine/DeviceManager.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"

namespace DAVA
{
namespace Private
{
DeviceManagerImpl::DeviceManagerImpl(DeviceManager* devManager, Private::MainDispatcher* dispatcher)
    : deviceManager(devManager)
    , mainDispatcher(dispatcher)
{
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_QT__

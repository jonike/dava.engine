#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/OsX/Window/WindowBackendOsX.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include <AppKit/NSScreen.h>

#include "Engine/OsX/WindowNativeServiceOsX.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/OsX/PlatformCoreOsX.h"
#include "Engine/Private/OsX/Window/WindowNativeBridgeOsX.h"

#include "Logger/Logger.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{
namespace Private
{
WindowBackend::WindowBackend(EngineBackend* e, Window* w)
    : engineBackend(e)
    , dispatcher(engineBackend->GetDispatcher())
    , window(w)
    , platformDispatcher(MakeFunction(this, &WindowBackend::EventHandler))
    , bridge(new WindowNativeBridge(this))
    , nativeService(new WindowNativeService(bridge))
{
}

WindowBackend::~WindowBackend()
{
    delete bridge;
}

void* WindowBackend::GetHandle() const
{
    return bridge->renderView;
}

bool WindowBackend::Create(float32 width, float32 height)
{
    hideUnhideSignalId = engineBackend->GetPlatformCore()->didHideUnhide.Connect(bridge, &WindowNativeBridge::ApplicationDidHideUnhide);

    NSSize screenSize = [[NSScreen mainScreen] frame].size;
    float32 x = (screenSize.width - width) / 2.0f;
    float32 y = (screenSize.height - height) / 2.0f;
    return bridge->DoCreateWindow(x, y, width, height);
}

void WindowBackend::Resize(float32 width, float32 height)
{
    UIDispatcherEvent e;
    e.type = UIDispatcherEvent::RESIZE_WINDOW;
    e.resizeEvent.width = width;
    e.resizeEvent.height = height;
    platformDispatcher.PostEvent(e);
}

void WindowBackend::Close()
{
    engineBackend->GetPlatformCore()->didHideUnhide.Disconnect(hideUnhideSignalId);

    UIDispatcherEvent e;
    e.type = UIDispatcherEvent::CLOSE_WINDOW;
    platformDispatcher.PostEvent(e);
}

bool WindowBackend::IsWindowReadyForRender() const
{
    return GetHandle() != nullptr;
}

void WindowBackend::RunAsyncOnUIThread(const Function<void()>& task)
{
    UIDispatcherEvent e;
    e.type = UIDispatcherEvent::FUNCTOR;
    e.functor = task;
    platformDispatcher.PostEvent(e);
}

void WindowBackend::TriggerPlatformEvents()
{
    bridge->TriggerPlatformEvents();
}

void WindowBackend::ProcessPlatformEvents()
{
    platformDispatcher.ProcessEvents();
}

bool WindowBackend::SetCaptureMode(eCaptureMode mode)
{
    if (eCaptureMode::FRAME == mode)
    {
        //for now, not supported
        return false;
    }
    UIDispatcherEvent e;
    e.type = UIDispatcherEvent::CHANGE_CAPTURE_MODE;
    e.mouseMode = mode;
    platformDispatcher.PostEvent(e);
    return true;
}

bool WindowBackend::SetMouseVisibility(bool visible)
{
    UIDispatcherEvent e;
    e.type = UIDispatcherEvent::CHANGE_MOUSE_VISIBILITY;
    e.mouseVisible = visible;
    platformDispatcher.PostEvent(e);
    return true;
}

void WindowBackend::EventHandler(const UIDispatcherEvent& e)
{
    switch (e.type)
    {
    case UIDispatcherEvent::RESIZE_WINDOW:
        bridge->DoResizeWindow(e.resizeEvent.width, e.resizeEvent.height);
        break;
    case UIDispatcherEvent::CLOSE_WINDOW:
        bridge->DoCloseWindow();
        break;
    case UIDispatcherEvent::FUNCTOR:
        e.functor();
        break;
    case UIDispatcherEvent::CHANGE_CAPTURE_MODE:
        bridge->ChangeCaptureMode(e.mouseMode);
        break;
    case UIDispatcherEvent::CHANGE_MOUSE_VISIBILITY:
        bridge->ChangeMouseVisibility(e.mouseVisible);
        break;
    default:
        break;
    }
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__

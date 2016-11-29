#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/EngineTypes.h"
#include "Engine/Private/EnginePrivateFwd.h"
#include "Engine/EngineTypes.h"

@class NSEvent;
@class NSWindow;

@class RenderView;
@class WindowDelegate;

namespace DAVA
{
namespace Private
{
// Bridge between C++ and Objective-C for OsX's WindowBackend class
// Responsibilities:
//  - holds neccesary Objective-C objects
//  - creates NSWindow
//  - processes notifications from WindowDelegate which implements
//    interface NSWindowDelegate
//  - posts events to dispatcher
//
// WindowNativeBridge is friend of OsX's WindowBackend
struct WindowNativeBridge final
{
    WindowNativeBridge(WindowBackend* windowBackend);
    ~WindowNativeBridge();

    bool CreateWindow(float32 x, float32 y, float32 width, float32 height);
    void ResizeWindow(float32 width, float32 height);
    void CloseWindow();
    void SetTitle(const char8* title);
    void SetMinimumSize(float32 width, float32 height);
    void SetFullscreen(eFullscreen newMode);
    float32 GetDpi();

    void TriggerPlatformEvents();

    void ApplicationDidHideUnhide(bool hidden);

    void WindowDidMiniaturize();
    void WindowDidDeminiaturize();
    void WindowDidBecomeKey();
    void WindowDidResignKey();
    void WindowDidResize();
    //void WindowWillStartLiveResize();
    //void WindowDidEndLiveResize();
    void WindowDidChangeScreen();
    bool WindowShouldClose();
    void WindowWillClose();
    void WindowWillEnterFullScreen();
    void WindowWillExitFullScreen();

    void MouseClick(NSEvent* theEvent);
    void MouseMove(NSEvent* theEvent);
    void MouseEntered(NSEvent* theEvent);
    void MouseExited(NSEvent* theEvent);
    void MouseWheel(NSEvent* theEvent);
    void KeyEvent(NSEvent* theEvent);
    void FlagsChanged(NSEvent* theEvent);
    void MagnifyWithEvent(NSEvent* theEvent);
    void RotateWithEvent(NSEvent* theEvent);
    void SwipeWithEvent(NSEvent* theEvent);

    void SetCursorCapture(eCursorCapture mode);
    void SetCursorVisibility(bool visible);
    static eModifierKeys GetModifierKeys(NSEvent* theEvent);
    static eMouseButtons GetMouseButton(NSEvent* theEvent);

    void SetSurfaceScale(const float32 scale);

    //////////////////////////////////////////////////////////////////////////

    WindowBackend* windowBackend = nullptr;
    Window* window = nullptr;
    MainDispatcher* mainDispatcher = nullptr;

    NSWindow* nswindow = nullptr;
    RenderView* renderView = nullptr;
    WindowDelegate* windowDelegate = nullptr;

    bool isAppHidden = false;
    bool isMiniaturized = false;
    bool isFullscreen;
    uint32 lastModifierFlags = 0; // Saved NSEvent.modifierFlags to detect Shift, Alt presses

private:
    void SetSystemCursorVisible(bool visible);
    void SetSystemCursorCapture(bool capture);

    eCursorCapture captureMode = eCursorCapture::OFF;
    bool mouseVisible = true;
    // If mouse pointer was outside window rectangle when enabling pinning mode then
    // mouse clicks are forwarded to other windows and our application loses focus.
    // So move mouse pointer to window center before enabling pinning mode.
    // Secondly, after using CGWarpMouseCursorPosition function to center mouse pointer
    // mouse move events arrive with big delta which causes mouse hopping.
    // The best solution I have investigated is to skip first N mouse move events after enabling
    // pinning mode: global variable mouseMoveSkipCount is set to some reasonable value
    // and is checked in OpenGLView's process method to skip mouse move events
    uint32 mouseMoveSkipCount = 0;
    const uint32 SKIP_N_MOUSE_MOVE_EVENTS = 4;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__

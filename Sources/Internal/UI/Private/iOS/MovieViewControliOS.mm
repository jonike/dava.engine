#include "UI/Private/iOS/MovieViewControliOS.h"

#if defined(__DAVAENGINE_IPHONE__)
#if !defined(DISABLE_NATIVE_MOVIEVIEW)

#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

#import <MediaPlayer/MediaPlayer.h>

#if defined(__DAVAENGINE_COREV2__)
#include "Engine/EngineModule.h"
#include "Engine/WindowNativeService.h"
#else
#import <Platform/TemplateiOS/HelperAppDelegate.h>
#endif

namespace DAVA
{
struct MovieViewControl::MovieViewObjcBridge final
{
    MPMoviePlayerController* moviePlayer = nullptr;
};

#if defined(__DAVAENGINE_COREV2__)
MovieViewControl::MovieViewControl(Window* w)
    : bridge(new MovieViewObjcBridge)
    , window(w)
#else
MovieViewControl::MovieViewControl()
    : bridge(new MovieViewObjcBridge)
#endif
{
    bridge->moviePlayer = [[MPMoviePlayerController alloc] init];

    [bridge->moviePlayer setShouldAutoplay:FALSE];
    [[bridge->moviePlayer view] setUserInteractionEnabled:NO];

    if ([bridge->moviePlayer respondsToSelector:@selector(loadState)])
    {
        [bridge->moviePlayer setControlStyle:MPMovieControlStyleNone];
        [bridge->moviePlayer setScalingMode:MPMovieScalingModeFill];
    }

#if defined(__DAVAENGINE_COREV2__)
    window->GetNativeService()->AddUIView([bridge->moviePlayer view]);
#else
    HelperAppDelegate* appDelegate = [[UIApplication sharedApplication] delegate];
    [[appDelegate renderViewController].backgroundView addSubview:[bridge->moviePlayer view]];
#endif
}

MovieViewControl::~MovieViewControl()
{
#if defined(__DAVAENGINE_COREV2__)
    window->GetNativeService()->RemoveUIView([bridge->moviePlayer view]);
#else
    [[bridge->moviePlayer view] removeFromSuperview];
#endif
    [bridge->moviePlayer release];
}

void MovieViewControl::Initialize(const Rect& rect)
{
    SetRect(rect);
}

void MovieViewControl::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
{
    NSURL* movieURL = [NSURL fileURLWithPath:[NSString stringWithCString:moviePath.GetAbsolutePathname().c_str() encoding:NSASCIIStringEncoding]];

    MPMovieScalingMode scaling = MPMovieScalingModeNone;
    switch (params.scalingMode)
    {
    case scalingModeNone:
        scaling = MPMovieScalingModeNone;
        break;
    case scalingModeFill:
        scaling = MPMovieScalingModeFill;
        break;
    case scalingModeAspectFill:
        scaling = MPMovieScalingModeAspectFill;
        break;
    case scalingModeAspectFit:
        scaling = MPMovieScalingModeAspectFit;
        break;
    default:
        scaling = MPMovieScalingModeNone;
        break;
    }
    [bridge->moviePlayer setScalingMode:scaling];
    [bridge->moviePlayer setContentURL:movieURL];
}

void MovieViewControl::SetRect(const Rect& rect)
{
    CGRect playerViewRect = [[bridge->moviePlayer view] frame];

    Rect physicalRect = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(rect);
    playerViewRect.origin.x = physicalRect.x + VirtualCoordinatesSystem::Instance()->GetPhysicalDrawOffset().x;
    playerViewRect.origin.y = physicalRect.y + VirtualCoordinatesSystem::Instance()->GetPhysicalDrawOffset().y;
    playerViewRect.size.width = physicalRect.dx;
    playerViewRect.size.height = physicalRect.dy;

#if defined(__DAVAENGINE_COREV2__)
    // Apply the Retina scale divider, if any.
    float32 scaleDivider = window->GetScaleX();
#else
    // Apply the Retina scale divider, if any.
    float32 scaleDivider = Core::Instance()->GetScreenScaleFactor();
#endif
    playerViewRect.origin.x /= scaleDivider;
    playerViewRect.origin.y /= scaleDivider;
    playerViewRect.size.height /= scaleDivider;
    playerViewRect.size.width /= scaleDivider;

    // Use decltype as CGRect::CGSize::width/height can be float or double depending on architecture 32-bit or 64-bit
    playerViewRect.size.width = std::max<decltype(playerViewRect.size.width)>(0.0, playerViewRect.size.width);
    playerViewRect.size.height = std::max<decltype(playerViewRect.size.width)>(0.0, playerViewRect.size.height);

    [[bridge->moviePlayer view] setFrame:playerViewRect];
}

void MovieViewControl::SetVisible(bool isVisible)
{
    [[bridge->moviePlayer view] setHidden:!isVisible];
}

void MovieViewControl::Play()
{
    [bridge->moviePlayer play];
}

void MovieViewControl::Stop()
{
    [bridge->moviePlayer stop];
}

void MovieViewControl::Pause()
{
    [bridge->moviePlayer pause];
}

void MovieViewControl::Resume()
{
    [bridge->moviePlayer play];
}

bool MovieViewControl::IsPlaying() const
{
    return [bridge->moviePlayer playbackState] == MPMoviePlaybackStatePlaying;
}

} // namespace DAVA

#endif // !DISABLE_NATIVE_MOVIEVIEW
#endif // __DAVAENGINE_IPHONE__

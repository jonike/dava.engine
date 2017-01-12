#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_IPHONE__)

#import <Foundation/NSObject.h>
#import "Engine/PlatformApiIos.h"

namespace DAVA
{
class LocalNotificationController;
}

@interface DVELocalNotificationListener : NSObject<DVEApplicationListener>

- (instancetype)initWithController:(DAVA::LocalNotificationController&)controller;

- (void)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions;
- (void)applicationDidBecomeActive:(NSNotification*)notification;
// - (void)userNotificationCenter:(NSUserNotificationCenter*)center didActivateNotification:(NSUserNotification*)notification;

@end

#endif // defined(__DAVAENGINE_MACOS__)
#endif // defined(__DAVAENGINE_COREV2__)

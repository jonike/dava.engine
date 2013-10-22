/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/
#include "Base/BaseTypes.h"
#include "Core/Core.h"
#if defined(__DAVAENGINE_IPHONE__)

#import "Platform/TemplateiOS/EAGLViewController.h"


@implementation EAGLViewController

/*
 // The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if (self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]) {
        // Custom initialization
    }
    return self;
}
*/
- (id) init
{
    if (self = [super init])
    {
        glView = nil;
        [self createGLView];
    }
    return self;
}

- (void) createGLView
{
    if (!glView)
    {
       glView = [[EAGLView alloc] initWithFrame:[[::UIScreen mainScreen] bounds]];
    }    
}

// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView 
{
	// To Hottych: Here should be proper initialization code ??? I mean, size of this view for iPhone / iPhone 4 / iPad
	// Check please what should be here
	[self createGLView];
    
    self.view = glView;
//	[glView release];
//   glView = nil;
}

/*
// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
}
*/

// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation 
{
    if (DAVA::Core::Instance()->GetScreenOrientation()==DAVA::Core::SCREEN_ORIENTATION_LANDSCAPE_AUTOROTATE)
        return (interfaceOrientation == UIInterfaceOrientationLandscapeLeft)||(interfaceOrientation == UIInterfaceOrientationLandscapeRight);
    if (DAVA::Core::Instance()->GetScreenOrientation()==DAVA::Core::SCREEN_ORIENTATION_PORTRAIT_AUTOROTATE)
        return (interfaceOrientation == UIInterfaceOrientationPortrait)||(interfaceOrientation == UIInterfaceOrientationPortraitUpsideDown);
    return FALSE;
}

-(BOOL)shouldAutorotate
{
    return (DAVA::Core::Instance()->GetScreenOrientation()==DAVA::Core::SCREEN_ORIENTATION_LANDSCAPE_AUTOROTATE)||(DAVA::Core::Instance()->GetScreenOrientation()==DAVA::Core::SCREEN_ORIENTATION_PORTRAIT_AUTOROTATE);
}
-(NSUInteger)supportedInterfaceOrientations
{
    if (DAVA::Core::Instance()->GetScreenOrientation()==DAVA::Core::SCREEN_ORIENTATION_LANDSCAPE_AUTOROTATE)
        return UIInterfaceOrientationMaskLandscape;
    if (DAVA::Core::Instance()->GetScreenOrientation()==DAVA::Core::SCREEN_ORIENTATION_PORTRAIT_AUTOROTATE)
        return UIInterfaceOrientationMaskPortrait;
    return FALSE;
    return UIInterfaceOrientationPortrait;
}


- (void)didReceiveMemoryWarning 
{
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload 
{
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}

- (void)dealloc 
{
    [glView release];
    glView = nil;
    [super dealloc];
}

- (void) viewWillAppear: (BOOL)animating
{
	NSLog(@"EAGLViewController viewWillAppear (startAnimation)");
	[glView setCurrentContext];
	[glView startAnimation];
}

- (void) viewDidDisappear: (BOOL)animating
{
	NSLog(@"EAGLViewController viewDidDisappear (stopAnimation)");
	[glView stopAnimation];
}


@end
#endif // 
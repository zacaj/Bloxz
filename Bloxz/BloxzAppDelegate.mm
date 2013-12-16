//
//  BloxzAppDelegate.m
//  Bloxz
//
//  Created by Zack Frey on 2/6/10.
//  Copyright Zision Games 2010. All rights reserved.
//

#import "BloxzAppDelegate.h"
#import "EAGLView.h"
#import "main.h"
#import "RockLicensing.h"
@implementation BloxzAppDelegate

@synthesize window;
@synthesize glView;

- (void) applicationDidFinishLaunching:(UIApplication *)application
{
	[glView startAnimation];
#if TARGET_IPHONE_SIMULATOR==0
	
	doCheckForAppWithName(@"Bloxz");
	
#endif
}

- (void) applicationWillResignActive:(UIApplication *)application
{
	[glView stopAnimation];
}

- (void) applicationDidBecomeActive:(UIApplication *)application
{
	[glView startAnimation];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	saveHighscore();
	[glView stopAnimation];
}

- (void) dealloc
{
	[window release];
	[glView release];
	
	[super dealloc];
}

@end

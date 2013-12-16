//
//  BloxzAppDelegate.h
//  Bloxz
//
//  Created by Zack Frey on 2/6/10.
//  Copyright Zision Games 2010. All rights reserved.
//

#import <UIKit/UIKit.h>

@class EAGLView;

@interface BloxzAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    EAGLView *glView;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet EAGLView *glView;

@end


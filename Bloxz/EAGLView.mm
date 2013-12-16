//
//  EAGLView.m
//  Bloxz
//
//  Created by Zack Frey on 2/6/10.
//  Copyright Zision Games 2010. All rights reserved.
//

#import "EAGLView.h"

#import "ES1Renderer.h"
#import "ES2Renderer.h"
#import "main.h"
@implementation EAGLView

@synthesize animating;
@dynamic animationFrameInterval;

// You must implement this method
+ (Class) layerClass
{
    return [CAEAGLLayer class];
}
extern int timeleft;
extern char *playername;
//The GL view is stored in the nib file. When it's unarchived it's sent -initWithCoder:
- (id) initWithCoder:(NSCoder*)coder
{    
    if ((self = [super initWithCoder:coder]))
	{
        // Get the layer
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        
        eaglLayer.opaque = TRUE;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
		
		//renderer = [[ES2Renderer alloc] init];
		
		if (!renderer)
		{
			renderer = [[ES1Renderer alloc] init];
			
			if (!renderer)
			{
				[self release];
				return nil;
			}
		}
        
		animating = FALSE;
		displayLinkSupported = FALSE;
		animationFrameInterval = 1;
		displayLink = nil;
		animationTimer = nil;
		
		// A system version of 3.1 or greater is required to use CADisplayLink. The NSTimer
		// class is used as fallback when it isn't available.
		NSString *reqSysVer = @"3.1";
		NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
		if ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending)
			displayLinkSupported = TRUE;
    }
    return self;
}

extern UITextField *textbox;
extern int menu;
extern int hst;
extern int gamemode;
extern int score,highscore[3];
- (void) drawView:(id)sender
{
	if(menu==2 && textbox==nil && score>getLow(gamemode-1))
	{
		textbox=[[UITextField alloc] initWithFrame:CGRectMake(138,195,150,30)];
		textbox.delegate=self;
		textbox.text=[[NSString alloc] initWithCString:playername];
		textbox.borderStyle=UITextBorderStyleNone;
	//	textbox.clearButtonMode=UITextFieldViewModeWhileEditing;
		textbox.textColor=[[UIColor alloc] initWithWhite:1 alpha:1];
		textbox.textAlignment=UITextAlignmentCenter;
		textbox.clearsOnBeginEditing=YES;
		
		[self addSubview:textbox];
	}
	else if(menu!=2 && textbox!=nil)
	{
		if(textbox.text.length==0)
			textbox.text=@"Player";
		playername=new char[textbox.text.length];
		strcpy(playername,[textbox.text cStringUsingEncoding:1]);
		[textbox removeFromSuperview];
		[textbox release];
		textbox=nil;
		addHighscore();
		newlevel();
	}
    [renderer render];
}
-(BOOL)textFieldShouldReturn:(UITextField *)textfield
{
	if(textfield.editing==YES)
	{
		[textbox resignFirstResponder];
		return YES;
	}
	else return NO;
}

- (void) layoutSubviews
{
	[renderer resizeFromLayer:(CAEAGLLayer*)self.layer];
    [self drawView:nil];
}

- (NSInteger) animationFrameInterval
{
	return animationFrameInterval;
}

- (void) setAnimationFrameInterval:(NSInteger)frameInterval
{
	// Frame interval defines how many display frames must pass between each time the
	// display link fires. The display link will only fire 30 times a second when the
	// frame internal is two on a display that refreshes 60 times a second. The default
	// frame interval setting of one will fire 60 times a second when the display refreshes
	// at 60 times a second. A frame interval setting of less than one results in undefined
	// behavior.
	if (frameInterval >= 1)
	{
		animationFrameInterval = frameInterval;
		
		if (animating)
		{
			[self stopAnimation];
			[self startAnimation];
		}
	}
}

- (void) startAnimation
{
	if (!animating)
	{
		if (displayLinkSupported)
		{
			// CADisplayLink is API new to iPhone SDK 3.1. Compiling against earlier versions will result in a warning, but can be dismissed
			// if the system version runtime check for CADisplayLink exists in -initWithCoder:. The runtime check ensures this code will
			// not be called in system versions earlier than 3.1.

			displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(drawView:)];
			[displayLink setFrameInterval:animationFrameInterval];
			[displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
		}
		else
			animationTimer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)((1.0 / 60.0) * animationFrameInterval) target:self selector:@selector(drawView:) userInfo:nil repeats:TRUE];
		
		animating = TRUE;
	}
}
- (void)stopAnimation
{
	if (animating)
	{
		if (displayLinkSupported)
		{
			[displayLink invalidate];
			displayLink = nil;
		}
		else
		{
			[animationTimer invalidate];
			animationTimer = nil;
		}
		
		animating = FALSE;
	}
}

- (void) dealloc
{
    [renderer release];
	
    [super dealloc];
}
extern int tx,ty,tim;
extern int firing;
-(void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	if(firing) return;
	NSSet *alltouches=[event allTouches];
	UITouch *touch=[[alltouches allObjects] objectAtIndex:0];
	CGPoint point=[touch locationInView:touch.view];
	if(menu==0)
	{
		if(point.x<32 && point.y>62 && point.y<316)
			fire(3,(point.y-30)/32,point.x);
		else if(point.x>32*9 && point.y>62 && point.y<316)
			fire(1,(point.y-30)/32,point.x);
		if(point.y<62 && point.x>32 && point.x<316)
			fire(0,point.x/32,point.y);
		else if(point.y>316 && point.x>32 && point.x<32*9 && point.y<390)
			fire(2,point.x/32,point.y);
		if(point.x<26 && point.y>454)
		{
			menu=1;
							   }
	}
	else if(menu==1)
	{
		if(point.x>160 && gamemode!=0)
			menu=0;
		else
		{
			if(point.y<330 && point.y>160)
			{
				menu=0;
				if(point.y<225)
					gamemode=1;
				else if(point.y<280)
					gamemode=2;
				else
					gamemode=3;
				newlevel();
			}
			if(point.y>360 && point.y<405)
				menu=3;
			if(point.y>415 && point.y<465)
			{
				menu=5;
				hst=0;
			}
		}
	}
	else if(menu==2)
	{
		if(point.y>320)
		{
			if(textbox==nil)
				newlevel();
			menu=1;
		}
		//[textbox dealloc];
	}
	else if(menu==3)
	{
		if(point.x>16 && point.x<48 && point.y>445 && point.y<477)
		{
			hst--;
			if(hst<0)
				hst=2;
		}
		else if(point.x>272 && point.x<304 && point.y>445 && point.y<477)
		{
			hst++;
			if(hst>2)
				hst=0;
		}
		else if(point.x>128 && point.x<192 && point.y>445 && point.y<477)
		{
			menu=1;
		}
	}
	else if(menu==4)
		menu=2;
	else if(menu==5)
	{
		if(point.x<26 && point.y>454)
		{
			menu=1;
		}
		else
		{
			hst++;
			if(hst>7)
				menu=1;
		}
	}
	printf("%f,%f\n",point.x,point.y);
	tx=ty=-1;
}
-(void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	if(firing) return;
	NSSet *alltouches=[event allTouches];
	UITouch *touch=[[alltouches allObjects] objectAtIndex:0];
	CGPoint point=[touch locationInView:touch.view];
	if(point.x<32 || point.x>320-32)
	{
		tx=-1;
		ty=(point.y-30)/32;
	}
	else if(point.y<62 || (point.y>316 && point.y<390))
	{
		ty=-1;
		tx=(point.x)/32;
	}
	else
	{
		tx=ty=-1;
	}
	if(tx==0 || tx==9)
		tx=-1;
	if(ty==0 || ty==9)
		ty=-1;
	tim=0;
}	
-(void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	if(firing) return;
	NSSet *alltouches=[event allTouches];
	UITouch *touch=[[alltouches allObjects] objectAtIndex:0];
	CGPoint point=[touch locationInView:touch.view];
	if(point.x<32 || point.x>320-32)
	{
		tx=-1;
		ty=(point.y-30)/32;
	}
	else if(point.y<62 || (point.y>316 && point.y<390))
	{
		ty=-1;
		tx=(point.x)/32;
	}
	else
	{
		tx=ty=-1;
	}
	if(tx==0 || tx==9)
		tx=-1;
	if(ty==0 || ty==9)
		ty=-1;
}	
@end

//
//  iphone_trdsAppDelegate.h
//  iphone_trds
//
//  Created by Bjørn Magnus Mathisen on 07.10.08.
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//

#import <UIKit/UIKit.h>

@class MyView;

@interface iphone_trdsAppDelegate : NSObject {
    UIWindow *window;
    MyView *contentView;
}

@property (nonatomic, retain) UIWindow *window;
@property (nonatomic, retain) MyView *contentView;

@end

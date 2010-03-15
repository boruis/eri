//
//  eagl_view.h
//  eri
//
//  Created by exe on 11/28/09.
//  Copyright 2009 cobbler. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface EAGLView : UIView
{
}

- (void)convertPointByViewOrientation:(CGPoint*)point;

@end

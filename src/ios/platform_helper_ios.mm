/*
 *  platform_helper.cpp
 *  eri
 *
 *  Created by exe on 11/28/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#include "platform_helper.h"

namespace ERI {
	
	void SetDeviceOrientation(ViewOrientation orientation)
	{
		switch (orientation) {
			case PORTRAIT_HOME_BOTTOM:
				[[UIApplication sharedApplication] setStatusBarOrientation: UIInterfaceOrientationPortrait animated:NO];
				break;
			case PORTRAIT_HOME_TOP:
				[[UIApplication sharedApplication] setStatusBarOrientation: UIInterfaceOrientationPortraitUpsideDown animated:NO];
				break;
			case LANDSCAPE_HOME_RIGHT:
				[[UIApplication sharedApplication] setStatusBarOrientation: UIInterfaceOrientationLandscapeRight animated:NO];
				break;
			case LANDSCAPE_HOME_LEFT:
				[[UIApplication sharedApplication] setStatusBarOrientation: UIInterfaceOrientationLandscapeLeft animated:NO];
				break;
			default:
				ASSERT(0);
				break;
		}
	}	
	
}

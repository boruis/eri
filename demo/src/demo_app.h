/*
 *  demo_app.h
 *  eri
 *
 *  Created by exe on 3/5/10.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#ifndef DEMO_APP_H
#define DEMO_APP_H

#include "input_mgr.h"

class DemoApp : ERI::InputHandler
	{
	public:
		DemoApp();
		~DemoApp();
		
		void Update(double delta_time);
		
		virtual void Press(const ERI::InputEvent& event);
		virtual void Release(const ERI::InputEvent& event);
		virtual void Click(const ERI::InputEvent& event);
	};

#endif // DEMO_APP_H

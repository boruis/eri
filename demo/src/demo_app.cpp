/*
 *  demo_app.cpp
 *  eri
 *
 *  Created by exe on 3/5/10.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#include "demo_app.h"

#include "root.h"
#include "renderer.h"
#include "scene_mgr.h"
#include "scene_actor.h"

ERI::CameraActor*	cam;
ERI::TxtActor*		hello_txt;
ERI::TxtActor*		fps_txt;
ERI::NumberActor*	fps_number;
ERI::SpriteActor*	pic;
ERI::SpriteActor*	pic_shadow;

DemoApp::DemoApp()
{
	ERI::Root::Ins().renderer()->SetBgColor(ERI::Color(0.2f, 0.1f, 0.2f));
	
	float content_scale = ERI::Root::Ins().renderer()->content_scale();
	
	int ui_layer = ERI::Root::Ins().scene_mgr()->AddLayer();
	
	cam = new ERI::CameraActor;
	cam->AddToScene(ui_layer);
	ERI::Root::Ins().scene_mgr()->SetCurrentCam(cam);
	
	hello_txt = new ERI::TxtActor("Hello! eri.", "georgia", static_cast<int>(32 * content_scale), true);
	hello_txt->AddToScene(ui_layer);
	hello_txt->SetTextureFilter(ERI::FILTER_LINEAR, ERI::FILTER_LINEAR);
	hello_txt->SetDepthWrite(false);
	hello_txt->SetPos(-50 * content_scale, 6 * content_scale);
	hello_txt->SetColor(ERI::Color(1.0f, 0.8f, 0.2f));
	
	fps_txt = new ERI::TxtActor("current fps is", "nokiafc22", static_cast<int>(16 * content_scale), true);
	fps_txt->AddToScene(ui_layer);
	fps_txt->SetPos(-110 * content_scale, -30 * content_scale);
	fps_txt->SetColor(ERI::Color(0.4f, 0.4f, 0.4f));
	
	fps_number = new ERI::NumberActor(20 * content_scale, 28 * content_scale, "media/num.png", 5, 7);
	fps_number->AddToScene(ui_layer);
	fps_number->SetPos(fps_txt->GetPos().x + static_cast<int>(fps_txt->width()), -30 * content_scale);
	fps_number->SetColor(ERI::Color(1, 0, 0));
	fps_number->SetNumberFloat(0.0f);
	
	pic = new ERI::SpriteActor(120 * content_scale, 120 * content_scale);
	pic->SetMaterial("media/eri.png", ERI::FILTER_LINEAR, ERI::FILTER_LINEAR);
	pic->AddToScene();
	pic->SetPos(ERI::Vector3(110 * content_scale, -5 * content_scale, -1));
	pic->SetRotate(-10);
	
	pic_shadow = new ERI::SpriteActor(132 * content_scale, 132 * content_scale);
	pic_shadow->SetMaterial("media/shadow.png", ERI::FILTER_LINEAR, ERI::FILTER_LINEAR);
	pic_shadow->AddToScene();
	pic_shadow->SetPos(ERI::Vector3(8 * content_scale, -8 * content_scale, -1));
	pic_shadow->SetColor(ERI::Color(1, 1, 1, 0.33f));
	pic->AddChild(pic_shadow);
}

DemoApp::~DemoApp()
{
	delete cam;
	delete hello_txt;
	delete fps_txt;
	delete fps_number;
	delete pic;
	//delete pic_shadow;
}

void DemoApp::Update(double delta_time)
{
	static int frame_count = 0;
	static double frame_count_timer = 0;
	
	++frame_count;
	frame_count_timer += delta_time;
	if (frame_count_timer >= 0.25)
	{
		fps_number->SetNumberFloat(static_cast<float>(frame_count / frame_count_timer));
		frame_count = 0;
		frame_count_timer = 0;
	}
}

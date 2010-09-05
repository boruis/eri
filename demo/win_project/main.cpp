#include "pch.h"

#include "win/framework_win.h"

#include "demo_app.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	ERI::FrameworkWin framework;
	framework.Init(hInstance, "demo", 480, 320);

	DemoApp app;

	float delta_time;
	framework.Run();
	while (framework.is_running())
	{
		delta_time = framework.PreUpdate();

		if (framework.IsVisible()) app.Update(delta_time);

		framework.PostUpdate();
	}

	framework.Release();

	return 0;
}
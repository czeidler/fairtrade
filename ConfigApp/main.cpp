/*
 * Copyright 2006-2013, Clemens Zeidler, czeidler@gmx.de
 * Distributed under the terms of the MIT License.              
 */

#include <Application.h>
#include <Window.h>

#include "ConfigFairTrade.h"


int 
main(int argc, char* argv[])
{
	BApplication app("application/x-vnd.Haiku-FairTradeConfig");
	
	BRect rect(20,20,300,300);
	ConfigWindow* window = new ConfigWindow(rect);
	ConfigFairTrade* configView = new ConfigFairTrade(window->Bounds());
	window->AddChild(configView);
	window->Show();
	
	app.Run();
	return 0;
}

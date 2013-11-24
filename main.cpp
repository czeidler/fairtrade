/*
 * Copyright 2006-2013, Clemens Zeidler, czeidler@gmx.de
 * Distributed under the terms of the MIT License.              
 */
 
#include "fairtrade.h"


int 
main(int argc, char* argv[])
{
	TradeApp *app = new TradeApp("application/x-vnd.Haiku-FairTrade");
	app->Run();
	delete app;
	return 0;
}

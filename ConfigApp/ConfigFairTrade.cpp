/*
 * Copyright 2006-2013, Clemens Zeidler, czeidler@gmx.de
 * Distributed under the terms of the MIT License.              
 */

#include <Roster.h>
#include <Path.h>
#include <Message.h>
#include <Button.h>
#include "blayout.h"

#define DEBUG 1
#include <Debug.h>

#include <unistd.h>

#include "ConfigFairTrade.h"


ConfigWindow::ConfigWindow(BRect rect)
	:
	BWindow(rect, "FairTrade Config", B_TITLED_WINDOW, B_NOT_RESIZABLE)
{
}

bool 
ConfigWindow::QuitRequested( void )
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}


ConfigFairTrade::ConfigFairTrade(BRect bounds)
	:
	BView(bounds, "ConfigFairTradeView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW| B_FRAME_EVENTS)
{
	app_info info;
	be_app->GetAppInfo(&info);
	BPath appPath;
	BEntry appEntry(&info.ref);
	appEntry.GetPath(&appPath);
	BString configPath = appPath.Path();
	
	configPath.RemoveLast(info.ref.name);
	chdir(configPath.String());
	
	fConfig.ReadConfig(kConfigFile);
}


ConfigFairTrade::~ConfigFairTrade()
{
}


void 
ConfigFairTrade::MessageReceived(BMessage *msg)
{
	switch (msg->what) {
		case WRITE_CONFIG:
			WriteConfig();
			break;
		default:
			BView::MessageReceived(msg);
	}
}


void 	
ConfigFairTrade::AttachedToWindow(void)
{
	//build gui
	BoxLayout *layout = new BoxLayout(Window()->Bounds(), "Layout", B_FOLLOW_ALL);
	fDatabaseEdit = new BTextControl(BRect(0,0,Bounds().Width(),10),"name","Database File:","",NULL);
	layout->AddView(fDatabaseEdit, 30);
	fExportPathEdit = new BTextControl(BRect(0,0,Bounds().Width(),10),"name","CSV export Path:","",NULL);
	layout->AddView(fExportPathEdit, 30);
	fWelcome1Edit = new BTextControl(BRect(0,0,Bounds().Width(),10),"name","First Welcome String:","",NULL);
	layout->AddView(fWelcome1Edit, 30);
	fWelcome2Edit = new BTextControl(BRect(0,0,Bounds().Width(),10),"name","Second Welcome String:","",NULL);
	layout->AddView(fWelcome2Edit, 30);
	fShutdownOnExitBox = new BCheckBox(BRect(0,0,Bounds().Width(),10), "checkbox", "Shutdown on exit", NULL);
	layout->AddView(fShutdownOnExitBox, 30);
	BButton* saveButton = new BButton(BRect(0,0,Bounds().Width(),10), "save Button", "Save Config", new BMessage(WRITE_CONFIG));
	saveButton->SetTarget(this);
	layout->AddView(saveButton, 30);
	layout->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(layout);
	
	BList controlList;
	controlList.AddItem(fDatabaseEdit);
	controlList.AddItem(fExportPathEdit);
	controlList.AddItem(fWelcome1Edit);
	controlList.AddItem(fWelcome2Edit);
	DivideSame(&controlList);
	
	fDatabaseEdit->SetText(fConfig.databasePath.String());
	fExportPathEdit->SetText(fConfig.csvExportPath.String());
	fWelcome1Edit->SetText(fConfig.welcomeString1.String());
	fWelcome2Edit->SetText(fConfig.welcomeString2.String());

	if (fConfig.shutdownOnExit)
		fShutdownOnExitBox->SetValue(B_CONTROL_ON);
}


void 
ConfigFairTrade::WriteConfig()
{
	BFile configFile(kConfigFile, B_READ_WRITE | B_CREATE_FILE);
	if (configFile.InitCheck() == B_NO_INIT)
		return;

	fConfig.databasePath = fDatabaseEdit->Text();
	fConfig.csvExportPath = fExportPathEdit->Text();
	fConfig.welcomeString1 = fWelcome1Edit->Text();
	fConfig.welcomeString2 = fWelcome2Edit->Text();
	fConfig.shutdownOnExit = fShutdownOnExitBox->Value();
	configFile.WriteAttr("databasePath", B_STRING_TYPE, 0, (void*)fConfig.databasePath.String(), 255);
	configFile.WriteAttr("csvExportPath", B_STRING_TYPE, 0, (void*)fConfig.csvExportPath.String(), 255);
	configFile.WriteAttr("welcomeString1", B_STRING_TYPE, 0, (void*)fConfig.welcomeString1.String(), 255);
	configFile.WriteAttr("welcomeString2", B_STRING_TYPE, 0, (void*)fConfig.welcomeString2.String(), 255);
	BString buffer;
	if (fConfig.shutdownOnExit)
		buffer = "1";
	else
		buffer = "0";
	configFile.WriteAttr("shutdownOnExit", B_STRING_TYPE, 0, (void*)buffer.String(), 255);
}


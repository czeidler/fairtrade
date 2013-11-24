/*
 * Copyright 2006-2013, Clemens Zeidler, czeidler@gmx.de
 * Distributed under the terms of the MIT License.              
 */

#ifndef CONFIGFAIRTRADE_H
#define CONFIGFAIRTRADE_H

#include <Application.h>
#include <Window.h>
#include <View.h>
#include <File.h>
#include <String.h>
#include <TextControl.h>
#include <CheckBox.h>

const char kConfigFile[255] = "FairTrade.cfg";
const int WRITE_CONFIG = '&WCF';


class config{
public:
	config()
	{
		databasePath = "./fairtrade.db";
		csvExportPath = "./CSVExport";
		welcomeString1 = "Willkommen";
		welcomeString2 = "bei der Eine-Welt-Gruppe Anrath";
		shutdownOnExit = true;
	}

	void ReadConfig(BString configPath)
	{
		BFile configFile;
		if(configFile.SetTo(configPath.String(), B_READ_ONLY) == B_ENTRY_NOT_FOUND){
			return;
		}
		char buffer[255];
		configFile.ReadAttr("databasePath", 0, 0, &buffer, 255);
		databasePath = buffer;
		configFile.ReadAttr("csvExportPath", 0, 0, &buffer, 255);
		csvExportPath = buffer;
		configFile.ReadAttr("welcomeString1", 0, 0, &buffer, 255);
		welcomeString1 = buffer;
		configFile.ReadAttr("welcomeString2", 0, 0, &buffer, 255);
		welcomeString2 = buffer;
		configFile.ReadAttr("shutdownOnExit", 0, 0, &buffer, 255);
		BString strToBool(buffer);
		if (strToBool.Compare("1") == 0)
			shutdownOnExit = true;
		else
			shutdownOnExit = false;
	}

	BString databasePath;
	BString csvExportPath;
	BString welcomeString1;
	BString welcomeString2;
	bool shutdownOnExit;
};

class ConfigWindow  : public BWindow 
{
public:
								ConfigWindow(BRect rect);

			bool				QuitRequested( void );
};

class ConfigFairTrade : public BView {
public:
								ConfigFairTrade(BRect rect);
	virtual						~ConfigFairTrade();
	virtual	void				MessageReceived(BMessage* msg);
	virtual void				AttachedToWindow(void);
				
private:
			void				WriteConfig();
		
			config				fConfig;
		
			BTextControl*		fDatabaseEdit;
			BTextControl*		fExportPathEdit;
			BTextControl*		fWelcome1Edit;
			BTextControl*		fWelcome2Edit;
			BCheckBox*	  		fShutdownOnExitBox;
};


#endif

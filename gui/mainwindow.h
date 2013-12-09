/*
 * Copyright 2006-2013, Clemens Zeidler, czeidler@gmx.de
 * Distributed under the terms of the MIT License.              
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Application.h>
#include <Bitmap.h>
#include <String.h>
#include <TranslationUtils.h>
#include <Window.h>

#include "ConfigFairTrade.h"
#include "mainview.h"
#include "productsview.h"
#include "sellview.h"
#include "stock.h"


const rgb_color kViewColor={20,200,0,0};
const rgb_color kBorderColor={20,180,0,0};

const float FONT_SIZE = 30.;
const float FONT_SIZE_BIG = 55.;

const uint BARCODE_ENTERED = '&bms';
const uint ESCAPE_PRESSED = '&esc';
const uint MANUAL_ENTER_REQUEST = '&mer';
const uint INVALID_PRODUCT = '&f9d';
const uint INVALID_REQUEST = '&inr';
const uint NEXT_BASKET = '&nba';
const uint PREV_BASKET = '&pba';
const uint SAVE_BASKET = '&sba';
const uint UPDATE_BASKET = '&uba';
const uint TRADE_TIME_EXPIRED = '&tte';

const uint8 NEXT_BASKET_KEY = B_RIGHT_ARROW;
const uint8 PREV_BASKET_KEY = B_LEFT_ARROW;
const uint8 SAVE_BASKET_KEY = B_SPACE;
const uint8 REMOVE_TRADE_KEY = B_BACKSPACE;

const int8 STACK_PRODUCT_VIEW = 0;
const int8 STACK_SELL_VIEW = 1;
const int8 STACK_MAIN_VIEW = 2;

const bigtime_t kTimerTime = 5000000;

class BitmapView : public BView
{
public:
								BitmapView(const char *filename);
								~BitmapView();
	virtual void				Draw(BRect updateRect);
	virtual void				GetPreferredSize(float *width, float *height);
private:
			BBitmap*			fBitmap;
};


class BarcodeFilter : public BMessageFilter
{
public:
								BarcodeFilter();
	virtual filter_result		Filter(BMessage *message, BHandler **target);

private:
			//read from barcodescanner:
			bool				fScannerEvent;
			bigtime_t			fScannerEventTime;
			BString				fBarcode;
};


class MainFilter : public BMessageFilter
{
public:
								MainFilter();
	virtual 					filter_result Filter(BMessage *message, BHandler **target);
};


class ManualBCInputWindow  : public BWindow 
{
public:
								ManualBCInputWindow(BRect rec, const char* title, BWindow *target);	
								~ManualBCInputWindow();
	virtual	void				MessageReceived(BMessage *msg);
private:
			BoxLayout*			fVLayout;
			BTextControl*		fBarcodeEdit;
			BMessenger*			fTarget;
};


class MainWindow  : public BWindow 
{
public:
								MainWindow(BRect, const char*, config &cfg);
								~MainWindow();
	virtual void 				WorkspaceActivated(int32 workspace, bool active);
	virtual	bool				QuitRequested( void );		
	virtual	void				MessageReceived(BMessage *msg);
						
	
			MainView*			fMainView;
			SellView*			fSellView;
			ProductView*		fProductView;
private:
			ManualBCInputWindow*	fManualBCInputWindow;
		
			YabStackView*		fViewStack;
		
			Timer*				fTradeTimer;	//empty status string
};


#endif

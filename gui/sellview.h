/*
 * Copyright 2006-2013, Clemens Zeidler, czeidler@gmx.de
 * Distributed under the terms of the MIT License.              
 */

#ifndef SELLVIEW_H
#define SELLVIEW_H

#include <ListView.h>
#include <ListItem.h>
#include <StringView.h>

#include "ConfigFairTrade.h"
#include "YabStackView.h"
#include "ColumnListView.h"
#include "ColumnTypes.h"
#include "stock.h"
#include "blayout.h"
#include "timer.h"

const int8 WELCOME_VIEW = 0;
const int8 SELL_VIEW = 1;

const int8 STATUS_VIEW = 0;
const int8 TRADE_SUCCESS_VIEW = 1;
const int8 TRADE_ABORT_VIEW = 2;

const rgb_color kModifiedColor = {200,200,0,0};
const rgb_color kSavedColor = {120,200,0,0};

const rgb_color kTradeAbortColor={240,0,0,0};
const rgb_color kTradeSuccessColor={250,150,0,0};
const rgb_color kBlackColor={0,0,0,0};
const rgb_color kNewTradeColor={0,100,250,0};

const char kSuccessString[255] = "Vielen Dank fuer Ihren Kauf.";
const char kAbortString[255] = "Handel wurde abgebrochen.";
const char kScanString[255] = "Bitte Produkt einscannen.";


class TradeRow : public BRow
{
public:
								TradeRow();
								~TradeRow();
								
			trade_info*			fTradeInfo;
			int8				fFirstNumber;
			int32				fLastKeyTime;
};


class SellListView : public BColumnListView
{
public:
	SellListView(BRect frame, const char *name)
		:BColumnListView(frame,name,B_FOLLOW_ALL,0)
	{
	}

	virtual void 				KeyDown(const char *bytes, int32 numBytes);
};


class SellView  : public BView 
{
public:
					 			SellView(BRect bounds, config &cfg);
					
	virtual void				FrameResized(float width, float height);
	virtual void				MakeFocus(bool focused);
		
			uint32				InitTradeTime();
			void				AddTrade(trade_info *tradeinfo);
			void				ShowPrevBasket();
			void				ShowNextBasket();
			void				ShowNewBasket();
			void				SaveCurrentBasket();
			void				UpdateCurrentBasket();		//redraw the current basket
			void				SetBasketsList(BList *list);
		
			void				UpdateTrade(trade_info *tradeInfo);
		
			YabStackView*		fStack;
			YabStackView*		fStatusStack;
			SellListView*		fTradeListView;
			BStringView*		fStatusString;
			basket_f*			fCurrentBasket;
		
private:
			void				ShowBasket(basket_f *basket);
		
				
			BoxLayout*			fWelcome;
			BStringView*		fWelcomeString;
			BStringView*		fScanString;
			BStringView*		fTradeSuccessString;
			BStringView*		fTradeAbortString;
		
			BColumn*			fNameColumn;
			BColumn*			fPrizeColumn;
			BColumn*			fSumColumn;
			BColumn*			fNumberColumn;
			BStringView*		fSumString;
			BStringView*		fIntroString;
			BStringView*		fDateString;
			BoxLayout*			fLayoutV;
					
			BList*				fBaskets;	
};



#endif

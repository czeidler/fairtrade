/*
 * Copyright 2006-2013, Clemens Zeidler, czeidler@gmx.de
 * Distributed under the terms of the MIT License.              
 */

#include "sellview.h"

#include <Font.h>
#include <Message.h>
#include <Messenger.h>


#define DEBUG 1
#include <Debug.h>

#include "fairtrade.h"
#include "mainwindow.h"


const uint NUMBER_COLUMN = 0;
const uint NAME_COLUMN = 1;
const uint PRIZE_COLUMN = 2;
const uint SUM_COLUMN = 3;

const char kSumString[255] = "Summe = ";
const char kIntroString[255] = "Verkauf vom ";


TradeRow::TradeRow()
{
	fFirstNumber = -1;
	fLastKeyTime = 0;
	
}


TradeRow::~TradeRow()
{

}

void
SellListView::KeyDown(const char *bytes, int32 numBytes)
{
	TradeRow *row = (TradeRow*)FocusRow();
	if(!row){
		return;
	}
	if (row && bytes[0] > 47 && bytes[0] < 58) {
		uint8 number;
		if(row->fFirstNumber > 0 && (real_time_clock() - row->fLastKeyTime) < 2){
			number = row->fFirstNumber * 10;
			number+= bytes[0] - 48;
			row->fFirstNumber = -1;
			row->fLastKeyTime = 0;
		}
		else
		{
			row->fLastKeyTime = real_time_clock();
			number = bytes[0] - 48;
			row->fFirstNumber = number;
		}
		BMessage msg(UPDATE_TRADE);
		msg.AddInt8("number",number);
		msg.AddPointer("tradeinfo",row->fTradeInfo);
		be_app->PostMessage(&msg);
		SetBackgroundColor(kModifiedColor);
	}
	if (bytes[0] == REMOVE_TRADE_KEY) {
		RemoveRow(row);
		BMessage msg(REMOVE_TRADE);
		msg.AddPointer("tradeinfo",row->fTradeInfo);
		be_app->PostMessage(&msg);
		SetBackgroundColor(kModifiedColor);
		delete row;
	}
	BColumnListView::KeyDown(bytes,numBytes);
}


SellView::SellView(BRect bounds, config &cfg)
	:
	BView(bounds, "mainview", B_FOLLOW_ALL_SIDES, B_WILL_DRAW| B_FRAME_EVENTS)
{
	fCurrentBasket = NULL;
	
	SetViewColor(kBorderColor);
	
	BRect mainRect = bounds;
	mainRect.InsetBy(20,20);
	
	fStack = new YabStackView(mainRect, "sell stack", 2 ,B_FOLLOW_ALL_SIDES);
	
	BFont font;
	font.SetSize(FONT_SIZE_BIG);
		
	//status
	fStatusString = new BStringView(BRect(bounds.left, 300, bounds.right, 400),
		"SS", kScanString);
	BFont statusFont;
	statusFont.SetSize(FONT_SIZE);
	fStatusString->SetFont(&statusFont);
	fStatusString->SetAlignment(B_ALIGN_CENTER);
	BoxLayout *statusLayout = new BoxLayout(bounds, "Layout status", B_FOLLOW_ALL);
	statusLayout->AddView(fStatusString, 100);
	statusLayout->SetViewColor(kViewColor);
	
	fTradeSuccessString = new BStringView(BRect(bounds.left, 300, bounds.right, 400),
		"SS", kSuccessString);
	statusFont.SetSize(FONT_SIZE_BIG);
	fTradeSuccessString->SetHighColor(kTradeSuccessColor);
	fTradeSuccessString->SetFont(&statusFont);
	fTradeSuccessString->SetAlignment(B_ALIGN_CENTER);
	BoxLayout *successLayout = new BoxLayout(bounds, "Layout success", B_FOLLOW_ALL);
	successLayout->AddView(fTradeSuccessString, 100);
	successLayout->SetViewColor(kViewColor);
	
	fTradeAbortString = new BStringView(BRect(bounds.left,300,bounds.right,400)
									, "AS", kAbortString);
	fTradeAbortString->SetHighColor(kTradeAbortColor);
	fTradeAbortString->SetFont(&statusFont);
	fTradeAbortString->SetAlignment(B_ALIGN_CENTER);
	BoxLayout *abortLayout = new BoxLayout(bounds, "Layout abort", B_FOLLOW_ALL);
	abortLayout->AddView(fTradeAbortString, 100);
	abortLayout->SetViewColor(kViewColor);
	
	fStatusStack = new YabStackView(bounds, "status stack", 3 ,B_FOLLOW_ALL_SIDES);
	BView **statusStackedViews = new BView*[3];
	statusStackedViews[STATUS_VIEW] = statusLayout;
	statusStackedViews[TRADE_SUCCESS_VIEW] = successLayout;
	statusStackedViews[TRADE_ABORT_VIEW] = abortLayout;
	fStatusStack->AddViews(statusStackedViews);
	delete[] statusStackedViews;
	//wellcome
	fWelcome = new BoxLayout(fStack->Bounds(),"LayoutV",B_FOLLOW_ALL);
	fWelcome->SetViewColor(kViewColor);
	fWelcomeString = new BStringView(BRect(bounds.left,100,bounds.right,200),
		"WS", cfg.welcomeString1.String());
	fWelcomeString->SetFont(&font);
	fWelcomeString->SetAlignment(B_ALIGN_CENTER);
	fWelcome->AddView(fWelcomeString,20);
	BStringView *anrathString  = new BStringView(BRect(bounds.left,100,bounds.right,200),
		"WS", cfg.welcomeString2.String());
	anrathString->SetFont(&font);
	anrathString->SetAlignment(B_ALIGN_CENTER);
	fWelcome->AddView(anrathString,20);
	fScanString = new BStringView(BRect(bounds.left,400,bounds.right,500),
		"SS",kScanString );
	font.SetSize(FONT_SIZE);
	fScanString->SetFont(&font);
	
	fScanString->SetAlignment(B_ALIGN_CENTER);
	fWelcome->AddView(fStatusStack,40);
	
	//picture bar
	font.SetSize(FONT_SIZE);
	BoxLayout *pictureLayout = new BoxLayout(fStack->Bounds(),"pictureH",B_FOLLOW_ALL);
	pictureLayout->SetViewColor(kBorderColor);
	pictureLayout->SetAlignment(HORIZONTAL);
	
	pictureLayout->AddView(new BitmapView("./pictures/left.png"), 15, true);
	pictureLayout->AddSpacer(70);
	pictureLayout->AddView(new BitmapView("./pictures/right.png"), 15, true);
	fWelcome->AddView(pictureLayout, 10);
	
	fTradeListView = new SellListView(BRect(0,0,100,100),"trade list");
	fTradeListView->SetFont(&font);
	fTradeListView->ShowScrollBar(false);
	fTradeListView->SetSelectionColor(kBorderColor);
	fTradeListView->SetSelectionMode(B_SINGLE_SELECTION_LIST);
	fTradeListView->SetBackgroundColor(kModifiedColor);
	fTradeListView->SetSortingEnabled(false);
	
	float listWidth = mainRect.Width();
	fNumberColumn = new BStringColumn("Anzahl", listWidth * 0.15, 10, listWidth,
		B_ALIGN_CENTER);
	fNameColumn = new BStringColumn("Produkt", listWidth * 0.55, 10, listWidth,
		B_ALIGN_LEFT);
	fPrizeColumn = new BStringColumn("Preis", listWidth * 0.15, 10, listWidth,
		B_ALIGN_RIGHT);
	fSumColumn = new BStringColumn("Summe", listWidth * 0.15, 10, listWidth,
		B_ALIGN_RIGHT);
	fTradeListView->AddColumn(fNumberColumn, NUMBER_COLUMN);
	fTradeListView->AddColumn(fNameColumn, NAME_COLUMN);
	fTradeListView->AddColumn(fPrizeColumn, PRIZE_COLUMN);
	fTradeListView->AddColumn(fSumColumn, SUM_COLUMN);
	
	//sum
	BoxLayout *sumLayout = new BoxLayout(fStack->Bounds(),"sumLayoutH",B_FOLLOW_ALL);
	sumLayout->SetAlignment(HORIZONTAL);
	sumLayout->SetViewColor(kViewColor);
	fSumString = new BStringView(BRect(0,10,mainRect.right,80)
									,"SuS","0", B_FOLLOW_RIGHT);
	font.SetSize(FONT_SIZE_BIG);
	fSumString->SetFont(&font);
	fSumString->SetAlignment(B_ALIGN_RIGHT);
	BStringView *constSumString = new BStringView(BRect(0,10,mainRect.right,80),"SuS",kSumString, B_FOLLOW_RIGHT);
	constSumString->SetFont(&font);
	sumLayout->AddSpacer(50);
	sumLayout->AddView(constSumString, 30);
	sumLayout->AddView(fSumString, 20);
			
	//Top intro string	
	font.SetSize(FONT_SIZE);	
	fIntroString = new BStringView(BRect(0,10,mainRect.right,80),
		"SuS",kIntroString, B_FOLLOW_RIGHT);
	fDateString = new BStringView(BRect(0,10,mainRect.right,80),
		"SuS"," date", B_FOLLOW_RIGHT);
	
	fIntroString->SetFont(&font);
	fDateString->SetFont(&font);
	fDateString->SetAlignment(B_ALIGN_RIGHT);
	BoxLayout *topLayout = new BoxLayout(fStack->Bounds(), "LayoutH",
		B_FOLLOW_ALL);
	topLayout->SetViewColor(kViewColor);
	topLayout->SetAlignment(HORIZONTAL);
	topLayout->AddView(fIntroString, 50);
	topLayout->AddView(fDateString, 50);
	
	//picture bar
	pictureLayout = new BoxLayout(fStack->Bounds(), "pictureH", B_FOLLOW_ALL);
	pictureLayout->SetViewColor(kBorderColor);
	pictureLayout->SetAlignment(HORIZONTAL);
	
	pictureLayout->AddView(new BitmapView("./pictures/esc.png"), 15, true);
	pictureLayout->AddView(new BitmapView("./pictures/space.png"), 60, true);
	pictureLayout->AddView(new BitmapView("./pictures/backspace.png"), 25, true);
	
	//mainlayout
	fLayoutV= new BoxLayout(fStack->Bounds(), "LayoutV", B_FOLLOW_ALL);
	//fSplitH->SetBarPosition(570);
	fLayoutV->SetViewColor(kViewColor);
	//fLayoutV->SetAlignment(HORIZONTAL);
	fLayoutV->AddView(topLayout, 7);
	fLayoutV->AddView(fTradeListView, 70);
	fLayoutV->AddView(sumLayout, 13);
	fLayoutV->AddView(pictureLayout, 10);

	BView **fStackedViews = new BView*[2];
	fStackedViews[0] = fWelcome;
	fStackedViews[1] = fLayoutV;
	fStack->AddViews(fStackedViews);
	delete[] fStackedViews;
	
	AddChild(fStack); 
	
}


void 
SellView::FrameResized(float width, float height)
{
	/*
	BRect mainRect = Frame();
	
	mainlistRect.InsetBy(mainlistRect.Width()/80,mainlistRect.Height()/80);
	fSellListView->ResizeTo(mainlistRect.Width(), mainlistRect.Height());
	fSellListView->MoveTo(mainlistRect.LeftTop());
	
	
	BFont font;
	font.SetSize(0.08*mainlistRect.Width());
	fSellListView->SetFont(&font);
	fSellListView->Invalidate();
	for (int i=0 ; i < fSellListView->CountItems(); i++){
		fSellListView->ItemAt(i)->Update(fSellListView, &font);
	}
	*/
}


void
SellView::MakeFocus(bool focused)
{
	fTradeListView->MakeFocus(true);
}


uint32 
SellView::InitTradeTime()
{
	if(fCurrentBasket == NULL){
		return real_time_clock();
	}
	return fCurrentBasket->date;
}


void 
SellView::AddTrade(trade_info *tradeinfo)
{
	fCurrentBasket = tradeinfo->basket;
	
	TradeRow *row = new TradeRow;
	row->fTradeInfo = tradeinfo;
	
	BString number;
	number << tradeinfo->trade->number;
	BStringField* sfield = new BStringField(number.String());
	row->SetField(sfield, NUMBER_COLUMN);
	
	sfield = new BStringField(tradeinfo->product->name.String());
	row->SetField(sfield, NAME_COLUMN);
	
	BString prize;
	prize << tradeinfo->trade->prize;
	sfield = new BStringField(prize.String());
	row->SetField(sfield, PRIZE_COLUMN);
			
	BString sumPart;
	sumPart << tradeinfo->trade->prize * tradeinfo->trade->number;
	sfield = new BStringField(sumPart.String());
	row->SetField(sfield, SUM_COLUMN);
	
	BString summe = "";
	summe << tradeinfo->basket->sum;
	fSumString->SetText(summe.String());
	fTradeListView->AddRow(row);
	fTradeListView->SetFocusRow(row,true);
	fTradeListView->SetBackgroundColor(kModifiedColor);
}


void 
SellView::UpdateTrade(trade_info *tradeInfo)
{
	TradeRow *row;
	for (int k = 0; ; k++) {
		row = (TradeRow*)fTradeListView->RowAt(k);
		if (row == NULL)
			break;
		
		if(row->fTradeInfo == tradeInfo){
			BStringField *stringfield;
			BString number;
			number << tradeInfo->trade->number;
			stringfield = (BStringField*)row->GetField(NUMBER_COLUMN);
			stringfield->SetString(number.String());
			
			BString prize;
			prize << tradeInfo->trade->prize;
			stringfield = (BStringField*)row->GetField(PRIZE_COLUMN);
			stringfield->SetString(prize.String());
			
			BString sumPart;
			sumPart << tradeInfo->trade->number * tradeInfo->trade->prize;
			stringfield = (BStringField*)row->GetField(SUM_COLUMN);
			stringfield->SetString(sumPart.String());
			
			fTradeListView->UpdateRow(row);
			
			BString sum;
			sum << tradeInfo->basket->sum;
			fSumString->SetText(sum.String());
			if (tradeInfo->basket->saved)
				fTradeListView->SetBackgroundColor(kSavedColor);
			else
				fTradeListView->SetBackgroundColor(kModifiedColor);
			return;
		} 
	}
}


void 
SellView::ShowPrevBasket()
{
	int32 n = fBaskets->IndexOf(fCurrentBasket);
	if (n < 0)
		n = fBaskets->CountItems();

	basket_f *basket = (basket_f*)fBaskets->ItemAt(n - 1);
	if (basket != NULL)
		ShowBasket(basket);
}


void 
SellView::ShowNextBasket()
{
	int32 n = fBaskets->IndexOf(fCurrentBasket);
	basket_f *basket = NULL;
	int32 nLastItem = fBaskets->CountItems() - 1;
	if (n + 1 <= nLastItem && n >= 0)
		basket = (basket_f*)fBaskets->ItemAt(n+1);

	ShowBasket(basket);		
}


void 
SellView::ShowNewBasket()
{
	ShowBasket(NULL);
}


void 
SellView::SaveCurrentBasket()
{
	BMessage msg(SAVE_TRADE);
	msg.AddPointer("basket", fCurrentBasket);
	be_app->PostMessage(&msg);
	fTradeListView->SetBackgroundColor(kSavedColor);
	fCurrentBasket = NULL;
}


void 
SellView::UpdateCurrentBasket()
{
	ShowBasket(fCurrentBasket);
}


void
SellView::SetBasketsList(BList *list)
{
	fBaskets = list;
}


void 
SellView::ShowBasket(basket_f *basket)
{
	//empty list
	BRow *row;
	while (true) {
		row = fTradeListView->RowAt(0);
		if (row == NULL)
			break;

		fTradeListView->RemoveRow(row);
		delete (TradeRow*)row;
	}
	//fill new list
	BString dateString = "";
	
	struct tm *structNow;
	if (basket)
		structNow = localtime(&(basket->date));
	else {
		time_t timeNow;
		time(&timeNow);
		structNow = localtime(&timeNow);
	}
	dateString << structNow->tm_mday; dateString+= ".";
	dateString << structNow->tm_mon + 1; dateString+= ".";
	dateString << structNow->tm_year + 1900; dateString+= " ";
	dateString << structNow->tm_hour; dateString+= ":";
	if (structNow->tm_min < 10)
		dateString << "0";
	dateString << structNow->tm_min; 
	
	fDateString->SetText(dateString.String());
	fSumString->SetText("0");
	
	fCurrentBasket = basket;
	if (basket == NULL) {
		fTradeListView->SetBackgroundColor(kModifiedColor);
		return;
	}
	//fill trades
	trade_info *info;
	for (int k = 0; ; k++) {
		info = (trade_info*)basket->trades.ItemAt(k);
		if (info == NULL)
			break;
		if (info->state != TRADE_STATE_REMOVE)
			AddTrade(info);
	}
	if (basket->saved)
		fTradeListView->SetBackgroundColor(kSavedColor);
	else
		fTradeListView->SetBackgroundColor(kModifiedColor);
}


/*
 * Copyright 2006-2013, Clemens Zeidler, czeidler@gmx.de
 * Distributed under the terms of the MIT License.              
 */

#include "mainwindow.h"

#include <Font.h>
#include <Message.h>
#include <Screen.h>
#include <SoundPlayer.h>
#include <Sound.h>
#include <Entry.h>
#include <scheduler.h>

#define DEBUG 1
#include <Debug.h>

const int32 PRE_POST_FIX = 92; //pre and postfix char


int32 
playsound(void *data){
	thread_id sender;
	char path[512];
	receive_data(&sender, (void *)path, sizeof(path));
	
	BSound *sound;
	BSoundPlayer player;
	entry_ref ref;
	BEntry entry(path, true);
	BSoundPlayer::play_id id;
	if (entry.InitCheck() == B_OK){
		if (entry.GetRef(&ref) == B_OK){
			sound = new BSound(&ref);
			if (sound->InitCheck() == B_OK){
				player.Start();
				player.SetVolume(1.0);
				id = player.StartPlaying(sound);
				sound->ReleaseRef();
				player.WaitForSound(id);
			}
		}
	}
	return 0;
}


BitmapView::BitmapView(const char *filename)
	:
	BView(BRect(0,0,10,10), "picture", B_FOLLOW_ALL, B_WILL_DRAW)
{
	fBitmap = BTranslationUtils::GetBitmapFile(filename);
	if (fBitmap)
		ResizeTo(fBitmap->Bounds().Width(), fBitmap->Bounds().Height());
}


BitmapView::~BitmapView()
{
	delete fBitmap;
}


void
BitmapView::Draw(BRect updateRect)
{
	if(fBitmap)
		DrawBitmap(fBitmap);
}


void 
BitmapView::GetPreferredSize(float *width, float *height)
{
	if (!fBitmap)
		return;
	(*width) = fBitmap->Bounds().Width();
	(*height) = fBitmap->Bounds().Height();
}


BarcodeFilter::BarcodeFilter()
	:BMessageFilter(B_ANY_DELIVERY,B_ANY_SOURCE)
{
	fScannerEvent = false;
	fScannerEventTime = 0;
	fBarcode = "";
}


filter_result 
BarcodeFilter::Filter(BMessage *message, BHandler **target)
{
	int32 key;
	switch (message->what){
		case B_KEY_DOWN:
			if (fScannerEvent){
				BString sKey;
				message->FindString("bytes",&sKey);
				PRINT(("barcode key: %s\n", sKey.String()));
				if (sKey.FindFirst("b") < 0)
					fBarcode += sKey;

				return B_SKIP_MESSAGE;
			}
		break;
		case  B_UNMAPPED_KEY_DOWN:
			message->FindInt32("key",&key);
			if ( key == PRE_POST_FIX ){ 
				PRINT(("unmapped barcode key: %i\n",key));
				//scanner event 
				if (fScannerEvent || fScannerEventTime + 50000 > system_time()){
					fScannerEvent = false;
					PRINT(("barcode readed: %s\n",fBarcode.String()));
					message->what = BARCODE_ENTERED;
					message->AddString("barcode",fBarcode);
					fBarcode = "";
				}
				else {
					fScannerEvent = true;
					fScannerEventTime = system_time();
				}
			}
			break;
	}
	return B_DISPATCH_MESSAGE;
}


MainFilter::MainFilter()
	:BMessageFilter(B_ANY_DELIVERY,B_ANY_SOURCE,B_KEY_DOWN)
{
	
}


filter_result 
MainFilter::Filter(BMessage *message, BHandler **target)
{
	int8 byte;
	message->FindInt8("byte", &byte);
	switch(byte) {
		case B_ESCAPE:
			message->what = ESCAPE_PRESSED;
			break;
		case NEXT_BASKET_KEY:
			Looper()->PostMessage(NEXT_BASKET);
			break;
		case PREV_BASKET_KEY:
			Looper()->PostMessage(PREV_BASKET);
			break;
		case SAVE_BASKET_KEY:
			Looper()->PostMessage(SAVE_BASKET);
			break;
	}
	int32 key;
	message->FindInt32("key", &key);
	switch (key) {
		case B_F2_KEY:
			message->what = MANUAL_ENTER_REQUEST;
			break;
		case B_F9_KEY:
			message->what = INVALID_REQUEST;
			break;
	}
				
	return B_DISPATCH_MESSAGE;
}


ManualBCInputWindow::ManualBCInputWindow(BRect rec, const char* title, BWindow *target)
		:BWindow(rec, title, B_BORDERED_WINDOW_LOOK, B_FLOATING_APP_WINDOW_FEEL, B_NOT_MOVABLE)
{
	//center window
	BScreen screen;
	BRect screenRect = screen.Frame();
	float xPosition = (screenRect.Width() - Bounds().Width()) / 2;
	float yPosition = (screenRect.Height() - Bounds().Height()) / 2;
	MoveTo(xPosition, yPosition);
	
	fTarget = new BMessenger(target);
	fBarcodeEdit = new BTextControl(BRect(0, 0, Bounds().Width(), 10), "name",
		"Barcode: ", "", NULL);
	fVLayout = new BoxLayout(Bounds(), "LayoutV", B_FOLLOW_ALL);
	fVLayout->AddView(fBarcodeEdit, 100);
	fVLayout->SetViewColor(kViewColor);

	BFont font;
	font.SetSize(FONT_SIZE);
	fBarcodeEdit->SetFont(&font);
	fBarcodeEdit->TextView()->SetFontAndColor(&font);
	AddChild(fVLayout);
	
	fBarcodeEdit->SetMessage(new BMessage(BARCODE_ENTERED));
	fBarcodeEdit->MakeFocus();
	
	Looper()->AddCommonFilter(new MainFilter());
}


ManualBCInputWindow::~ManualBCInputWindow()
{
	delete fTarget;
}


void
ManualBCInputWindow::MessageReceived(BMessage *msg)
{
	BMessage barcodemsg;
	switch (msg->what) {
		case BARCODE_ENTERED:
			barcodemsg.what = BARCODE_ENTERED;
			barcodemsg.AddString("barcode",fBarcodeEdit->Text());
			fTarget->SendMessage(&barcodemsg);
			Quit();
			break;
		case ESCAPE_PRESSED:
			Quit();
			break;
	}
}


MainWindow::MainWindow(BRect rec, const char* title, config &cfg)
	:
	BWindow(rec, title, B_NO_BORDER_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL,
		B_NOT_MOVABLE)
{	
	BScreen screen;
	BRect screenRect = screen.Frame();
	MoveTo(0,0);
	ResizeTo(screenRect.Width(),screenRect.Height());
	
	BRect bounds = Bounds();
	fProductView = new ProductView(bounds);
	fSellView = new SellView(bounds, cfg);
	fMainView = new MainView(bounds);
	
	fViewStack = new YabStackView(bounds, "view stack", 3 ,B_FOLLOW_ALL_SIDES);
	BView **stackedViews = new BView*[3];
	stackedViews[0] = fProductView;
	stackedViews[1] = fSellView;
	stackedViews[2] = fMainView;
	fViewStack->AddViews(stackedViews);
	delete[] stackedViews;
	
	AddChild(fViewStack);
	fViewStack->SelectView(STACK_SELL_VIEW);
	
	Looper()->AddCommonFilter(new MainFilter());
	Looper()->AddCommonFilter(new BarcodeFilter());
		
	fTradeTimer = new Timer;
	fTradeTimer->SetTarget(this);
	fTradeTimer->SetMessage(new BMessage(TRADE_TIME_EXPIRED));
}


MainWindow::~MainWindow()
{
	// TODO: there is a race condition when the timer thread is still running
	//delete fTradeTimer;
}


void 
MainWindow::MessageReceived(BMessage *msg)
{
	BListView *mainmenu = NULL;
	MainListItem *mainmenuitem = NULL;
	BString barcode;
	product_f *product;
	trade_info *tradeinfo;
	ProductRow *row;
	//for scan sound:
	thread_id thread;
	int32 code = 63;
	char *buf = "./klack";
			
	switch(msg->what) {
		case MAIN_MENU_MSG:
			if (msg->FindPointer("source", (void**)&mainmenu) == B_OK) {
				int32 index;
				msg->FindInt32("index", &index);

				mainmenuitem = (MainListItem*)mainmenu->ItemAt(index);
				if (mainmenuitem && mainmenuitem->fMenuItem == MAIN_MENU_SELL) {
					be_app->HideCursor();
					fViewStack->SelectView(STACK_SELL_VIEW);
					fSellView->MakeFocus(true);
				}
				if (mainmenuitem && mainmenuitem->fMenuItem == MAIN_MENU_PRODUCTS) {
					be_app->ShowCursor();
					fViewStack->SelectView(STACK_PRODUCT_VIEW);
					fProductView->MakeFocus(true);
				}
				if (mainmenuitem && mainmenuitem->fMenuItem == MAIN_MENU_EXIT)
					be_app->PostMessage(QUIT);
			}
			break;
		case ADD_PRODUCT:
			msg->FindPointer("product", (void**)&product);
			fProductView->AddProduct(product);
			break;
		case UPDATE_PRODUCT:
			msg->FindPointer("product", (void**)&product);
			fProductView->UpdateProduct(product);
			fSellView->UpdateCurrentBasket();
			break;
		case ADD_TRADE:
			msg->FindPointer("tradeinfo", (void**)&tradeinfo);
			fSellView->AddTrade(tradeinfo);
			break;
		case UPDATE_TRADE:
			msg->FindPointer("tradeinfo", (void**)&tradeinfo);
			fSellView->UpdateTrade(tradeinfo);
			break;
		case PRODUCT_INVOKED:
			row = (ProductRow*)fProductView->fProductListView->CurrentSelection();
			if(row)
				fProductView->EditProduct(row->product);
			break;
		case ESCAPE_PRESSED:
			if(!fProductView->IsHidden()){
				fViewStack->SelectView(STACK_MAIN_VIEW);
				fMainView->MakeFocus(true);
			}
			if (!fSellView->IsHidden()) {
				if (fSellView->fStack->CurrentView() == SELL_VIEW) {
					fSellView->fStatusStack->SelectView(TRADE_ABORT_VIEW);
					fSellView->fCurrentBasket = NULL;
					fTradeTimer->Start(kTimerTime);
					fSellView->fStack->SelectView(WELCOME_VIEW);
				} else {
					fViewStack->SelectView(STACK_MAIN_VIEW);
					be_app->ShowCursor();
					fMainView->MakeFocus(true);
				}
			}
			break;
		case MANUAL_ENTER_REQUEST:
			if (!fSellView->IsHidden() || !fProductView->IsHidden())
			{
				fManualBCInputWindow = new ManualBCInputWindow(BRect(300, 200, 800, 300), "Barcode Eingabe", this);
				fManualBCInputWindow->Show();
			}
			break;
		case BARCODE_ENTERED:
			thread = spawn_thread(playsound, "scanSound", B_AUDIO_PLAYBACK, 0);
			send_data(thread, code, (void*)buf, strlen(buf));
			resume_thread(thread);
			
			if (!fProductView->IsHidden()) {
				msg->FindString("barcode", &barcode);
				fProductView->BarcodeEntered(barcode);
			}
			if (!fSellView->IsHidden()) {
				if(fSellView->fStack->CurrentView() == WELCOME_VIEW){
					fSellView->fStack->SelectView(SELL_VIEW);
					fSellView->fTradeListView->MakeFocus(true);
					fSellView->ShowNewBasket();
				}
				msg->what = SELL_PRODUCT;
				msg->AddInt32("tradetime",fSellView->InitTradeTime());
				be_app->PostMessage(msg);
			}
			break;
		case NEXT_BASKET:
			if (!fSellView->IsHidden()) {
				if(fSellView->fStack->CurrentView() == WELCOME_VIEW){
					fSellView->fStack->SelectView(SELL_VIEW);
					fSellView->ShowNewBasket();
					fSellView->fTradeListView->MakeFocus(true);
				}
				else{
					fSellView->ShowNextBasket();
				}
			}
			break;
		case PREV_BASKET:
			if (!fSellView->IsHidden()) {
				if(fSellView->fStack->CurrentView() == WELCOME_VIEW){
					fSellView->fStack->SelectView(SELL_VIEW);
					fSellView->fTradeListView->MakeFocus(true);
				}
				fSellView->ShowPrevBasket();
			}
			break;
		case SAVE_BASKET:
			if (!fSellView->IsHidden() && fSellView->fStack->CurrentView() == SELL_VIEW) {
				fSellView->SaveCurrentBasket();
				fSellView->fStatusStack->SelectView(TRADE_SUCCESS_VIEW);
				fTradeTimer->Start(kTimerTime);
				fSellView->fStack->SelectView(WELCOME_VIEW);
			}
			break;
		case UPDATE_BASKET:
			fSellView->UpdateCurrentBasket();
			break;
		case TRADE_TIME_EXPIRED:
			fSellView->fStatusStack->SelectView(STATUS_VIEW);
			break;
		default:
			BWindow::MessageReceived(msg);
	}
	
}


void 	
MainWindow::WorkspaceActivated(int32 workspace, bool active)
{
	/*if(active){
		Activate();
		if(!fSellView->IsHidden()){
			fSellView->fTradeListView->MakeFocus(true);
		}
		if(!fSellView->IsHidden()){
			fProductView->fProductListView->MakeFocus(true);
		}
	}*/
}


bool 
MainWindow::QuitRequested(void)
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}



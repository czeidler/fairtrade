/*
 * Copyright 2006-2013, Clemens Zeidler, czeidler@gmx.de
 * Distributed under the terms of the MIT License.              
 */

#include "mainview.h"

#include <Font.h>
#include <Message.h>

#include "mainwindow.h"


MainListItem::MainListItem(int8 menuitem, const char *text)
							:BStringItem(text){
	fMenuItem = menuitem;
}


MainView::MainView(BRect bounds)
	:
	BView(bounds, "mainview", B_FOLLOW_ALL_SIDES, B_WILL_DRAW| B_FRAME_EVENTS)
{
	SetViewColor(kBorderColor);
	BRect mainlistRect = bounds;
	mainlistRect.InsetBy(20,30);
	fMainMenuListView = new MainListView(mainlistRect, "tradeview");
	
	BMessage *message = new BMessage(MAIN_MENU_MSG);
	fMainMenuListView->SetInvocationMessage(message);
	BFont font;
	font.SetSize(60.0);
	fMainMenuListView->SetFont(&font);
	
	fMainMenuListView->AddItem(new MainListItem(MAIN_MENU_SELL,"Verkauf"));
	fMainMenuListView->AddItem(new MainListItem(MAIN_MENU_PRODUCTS,"Produkt Uebersicht"));
	fMainMenuListView->AddItem(new MainListItem(MAIN_MENU_EXIT,"Computer ausschalten"));
	AddChild(fMainMenuListView); 
	
	fMainMenuListView->MakeFocus(true);
}


void 
MainView::FrameResized(float width, float height)
{
	BRect mainlistRect = Frame();
	mainlistRect.InsetBy(mainlistRect.Width()/80,mainlistRect.Height()/80);
	fMainMenuListView->ResizeTo(mainlistRect.Width(), mainlistRect.Height());
	fMainMenuListView->MoveTo(mainlistRect.LeftTop());
	
	BFont font;
	font.SetSize(0.08*mainlistRect.Width());
	fMainMenuListView->SetFont(&font);
	fMainMenuListView->Invalidate();
	for (int i=0 ; i < fMainMenuListView->CountItems(); i++){
		fMainMenuListView->ItemAt(i)->Update(fMainMenuListView, &font);
	}
}


void
MainView::MakeFocus(bool focused)
{
	fMainMenuListView->MakeFocus(true);
}


void MainListView::KeyDown(const char *bytes, int32 numBytes)
{
	//PRINT(("Input: %s\n"));
	BListView::KeyDown(bytes,numBytes);
}





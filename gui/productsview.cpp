/*
 * Copyright 2006-2013, Clemens Zeidler, czeidler@gmx.de
 * Distributed under the terms of the MIT License.              
 */
 
#include "productsview.h"

#include <Font.h>
#include <Rect.h>
#include <Message.h>
#include <Alert.h>
#include <Screen.h>

#include "fairtrade.h"
#include "mainwindow.h"


const float ALIGN_LEFT = 50;
const float ALIGN_RIGHT = 50;
const float ALIGN_TOP = 50;
const float SPACE = 100;

const uint NAME_COLUMN = 0;
const uint PRIZE_COLUMN = 1;
const uint SUPPLIES_COLUMN = 2;

#define DEBUG 1
#include <Debug.h>


ProductView::ProductView(BRect bounds)
		:
		BView(bounds, "productview", B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS)
{
	SetViewColor(kBorderColor);
	BRect mainlistRect = bounds;
	mainlistRect.InsetBy(20,30);
	
	BFont font;
	font.SetSize(FONT_SIZE);								
	fProductListView = new BColumnListView(mainlistRect,"product list",B_FOLLOW_ALL,0);
	
	fNameColumn = new BStringColumn("Produkt",650,15,800,255);
	fPrizeColumn = new BStringColumn("Preis",140,15, 500,255);
	fSuppliesColumn = new BIntegerColumn("Vorrat",140,15,500);
	fProductListView->AddColumn(fNameColumn,NAME_COLUMN);
	fProductListView->AddColumn(fPrizeColumn,PRIZE_COLUMN);
	fProductListView->AddColumn(fSuppliesColumn,SUPPLIES_COLUMN);
	
	fProductListView->SetSortingEnabled(true);
	fProductListView->SetSortColumn(fSuppliesColumn, true,true);
	
	fProductListView->SetFont(&font);
	fProductListView->SetColor(B_COLOR_BACKGROUND, kSavedColor);
	fProductListView->ShowScrollBar(false);
	
	fProductListView->SetInvocationMessage(new BMessage(PRODUCT_INVOKED));
	fProductListView->SetSelectionMode(B_SINGLE_SELECTION_LIST);
	fProductListView->SetSelectionColor(kBorderColor);
	
	AddChild(fProductListView); 
}


void 
ProductView::FrameResized(float width, float height)
{
	BRect mainlistRect = Frame();
	mainlistRect.InsetBy(mainlistRect.Width()/80,mainlistRect.Height()/80);
	fProductListView->ResizeTo(mainlistRect.Width(), mainlistRect.Height());
	fProductListView->MoveTo(mainlistRect.LeftTop());
}


void
ProductView::MakeFocus(bool focused)
{
	fProductListView->MakeFocus(true);
}


void	
ProductView::AddProduct(product_f *product)
{
	if (product->valid) {
		ProductRow *row = new ProductRow;
		row->product = product;
	
		BStringField* sfield = new BStringField(row->product->name.String());
		row->SetField(sfield, NAME_COLUMN);
	
		BString prize;
		prize << row->product->prize;
		sfield = new BStringField(prize.String());
		row->SetField(sfield, PRIZE_COLUMN);
	
		BIntegerField *ifield = new BIntegerField(product->supplies);
		row->SetField(ifield, SUPPLIES_COLUMN);
	
		fProductListView->AddRow(row);
	} else
		fUnvalidList.AddItem(product);
}


void 
ProductView::BarcodeEntered(BString barcode)
{
	bool isinlist = false;
	bool isInUnvalidList = false;
	ProductRow *row;
	for(int i = 0; i < fProductListView->CountRows(); i++)
	{
		row = (ProductRow*)fProductListView->RowAt(i);
		if(row->product->barcode == barcode){
			fProductListView->SetFocusRow(row,true);
			fProductListView->ScrollTo(row);
			EditProduct(row->product);
			isinlist = true;
			break;
		}
	}
	if(!isinlist){
		//check if it is in the unvalid list
		for(int i = 0; i < fUnvalidList.CountItems(); i++)
		{
			product_f* product = (product_f*)fUnvalidList.ItemAt(i);
			if(product->barcode == barcode){
				//valid item
				product->valid = true;
				//update store
				BMessage reply;
				BMessage msg(SAVE_PRODUCT);
				product->Archive(&msg);
				if(be_app_messenger.SendMessage(&msg,&reply) == B_OK){
					//update list
					isInUnvalidList = true;
					EditProduct(product);
				}				
				break;
			}
		}
	}
	if(!isInUnvalidList && !isinlist){
		product_f product;
		product.barcode = barcode;
		product.prize = 0;
		product.supplies = 0;
		EditProduct(&product);
	}
}


void	
ProductView::EditProduct(product_f *product = NULL)
{
	BWindow *EditWindow = new EditProductWindow(BRect(150,200,700,700),"edit product",product);
	EditWindow->Show();
}


void	
ProductView::UpdateProduct(product_f *product)
{
	ProductRow *row;
	for (int k = 0; ; k++) {
		row = (ProductRow*)fProductListView->RowAt(k);
		if (row == NULL)
			break;
		
		if (row->product == product) {
			if (product->valid == false) {
				fProductListView->RemoveRow(row);
				fUnvalidList.AddItem(product);
				return;
			}
			BStringField *stringfield;
			stringfield = (BStringField*)row->GetField(NAME_COLUMN);
			stringfield->SetString(product->name.String());
			BString prize;
			prize << product->prize;
			stringfield = (BStringField*)row->GetField(PRIZE_COLUMN);
			stringfield->SetString(prize.String());
			BIntegerField *ifield = (BIntegerField*)row->GetField(SUPPLIES_COLUMN);
			ifield->SetValue(product->supplies);
			
			fProductListView->UpdateRow(row);
			return;
		} 
	}
	
	for (int i = 0; i < fUnvalidList.CountItems(); i++)
	{
		product_f* prod = (product_f*)fUnvalidList.ItemAt(i);
		if (prod == product && prod->valid) {
			fUnvalidList.RemoveItem(i);
			AddProduct(product);	
			break;
		}
	}
	
}


status_t 
EditProductWindow::GetProductInfo(product_f *pinfo)
{
	pinfo->barcode = fBarcodeEdit->Text();
	pinfo->name = fNameEdit->Text();
	pinfo->prize = atof(fPrizeEdit->Text());
	pinfo->comment = fCommentEdit->Text();
	pinfo->valid = true;
	
	//PRINT(("supplies%i, %i\n",atoi(fSuppliesEdit->Text()), fProduct->supplies));
	pinfo->supplies = atoi(fSuppliesEdit->Text());
	if(atoi(fSuppliesEdit->Text()) == fInitialSupplies){
		pinfo->supplies+= atoi(fAddSuppliesEdit->Text());
	}
	if(pinfo->name != "" && pinfo->barcode != ""){
		return B_OK;
	}
	return B_ERROR;
}


EditProductWindow::EditProductWindow(BRect rec, const char* title, product_f *product, bool sell, int32 tradeTime)
			 :BWindow(rec, title, B_BORDERED_WINDOW_LOOK, B_FLOATING_APP_WINDOW_FEEL, B_NOT_MOVABLE)
			 , fSell(sell), fTradeTime(tradeTime)
{
	//center window
	BScreen screen;
	BRect screenRect = screen.Frame();
	float xPosition = (screenRect.Width() - Bounds().Width()) / 2;
	float yPosition = (screenRect.Height() - Bounds().Height()) / 2;
	MoveTo(xPosition, yPosition);
	
	fBarcodeEdit = new BTextControl(BRect(0,0,rec.Width(),10),"name","Barcode","",NULL);
	fBarcodeEdit->SetEnabled(false);
	fNameEdit = new BTextControl(BRect(0,0,Bounds().Width(),10),"name","Produkt Name","",NULL);
	fPrizeEdit = new BTextControl(BRect(0,0,Bounds().Width(),10),"name","Preis","",NULL);
	fPrizeEdit->SetModificationMessage(new BMessage(NUMBER_MODIFIED));
	fCommentEdit = new BTextControl(BRect(0,0,Bounds().Width(),10),"name","Beschreibung","",NULL);
	fAddSuppliesEdit = new BTextControl(BRect(0,0,Bounds().Width(),10),"name","Wareneingang","",NULL);
	fAddSuppliesEdit->SetModificationMessage(new BMessage(NUMBER_MODIFIED));
	fSuppliesEdit = new BTextControl(BRect(0,0,rec.Width(),10),"name","Bestand","",NULL);
	fSuppliesEdit->SetModificationMessage(new BMessage(NUMBER_MODIFIED));
	fOkButton = new BButton(BRect(0,0,rec.Width(),10),"name","Produkt speichern",new BMessage(EDIT_PRODUCT));
	
	fHLayout = new BoxLayout(BRect(0,0,10,10),"LayoutH",B_FOLLOW_ALL);
	fHLayout->SetAlignment(HORIZONTAL);
	fHLayout->SetViewColor(kViewColor);
	fHLayout->AddView(fBarcodeEdit, 65);
	fHLayout->AddView(fSuppliesEdit, 35);
	
	//picture bar
	BoxLayout *pictureLayout = new BoxLayout(Bounds(),"pictureH",B_FOLLOW_ALL);
	pictureLayout->SetViewColor(kViewColor);
	pictureLayout->SetAlignment(HORIZONTAL);
	pictureLayout->AddView(new BitmapView("./pictures/tab.png"), 50, true);
	if(!fSell){
		pictureLayout->AddView(new BitmapView("./pictures/productEntf.png"), 50, true);
	}
	
	fProductLayout = new BoxLayout(Bounds(),"LayoutV",B_FOLLOW_ALL);
	fProductLayout->SetViewColor(kViewColor);
	
	fProductLayout->AddView(fHLayout, 30);
	fProductLayout->AddView(fNameEdit, 30);
	fProductLayout->AddView(fPrizeEdit, 30);
	fProductLayout->AddView(fCommentEdit, 30);
	fProductLayout->AddView(fAddSuppliesEdit, 30);
	fProductLayout->AddView(fOkButton, 30);
	fProductLayout->AddView(pictureLayout, 30);
	
	Looper()->AddCommonFilter(new MainFilter());
	
	BFont font;
	font.SetSize(FONT_SIZE);
	fBarcodeEdit->SetFont(&font);
	fBarcodeEdit->TextView()->SetFontAndColor(&font);
	//fBarcodeEdit->ResizeToPreferred();
	fNameEdit->SetFont(&font);
	fNameEdit->TextView()->SetFontAndColor(&font);
	//fNameEdit->ResizeToPreferred();
	fPrizeEdit->SetFont(&font);
	fPrizeEdit->TextView()->SetFontAndColor(&font);
	//fPrizeEdit->ResizeToPreferred();
	fCommentEdit->SetFont(&font);
	fCommentEdit->TextView()->SetFontAndColor(&font);
	//fCommentEdit->ResizeToPreferred();
	
	fAddSuppliesEdit->SetFont(&font);
	fAddSuppliesEdit->TextView()->SetFontAndColor(&font);
	//fAddSuppliesEdit->ResizeToPreferred();
	fSuppliesEdit->SetFont(&font);
	fSuppliesEdit->TextView()->SetFontAndColor(&font);
	//fSuppliesEdit->ResizeToPreferred();
	//DivideSame(fNameEdit,fPrizeEdit,fCommentEdit,fAddSuppliesEdit,0);	
	fOkButton->SetFont(&font);
	//fOkButton->ResizeToPreferred();
	
	AddChild(fProductLayout); 
	BList controlList;
	controlList.AddItem(fNameEdit);
	controlList.AddItem(fPrizeEdit);
	controlList.AddItem(fCommentEdit);
	controlList.AddItem(fAddSuppliesEdit);
	DivideSame(&controlList);
	
	//fill the fields
	fInitialSupplies = product->supplies;
	fBarcodeEdit->SetText(product->barcode.String());
	fNameEdit->SetText(product->name.String());
	BString prize;
	prize << product->prize;
	fPrizeEdit->SetText(prize.String());
	fCommentEdit->SetText(product->comment.String());
	BString supplies;
	supplies << product->supplies;
	fSuppliesEdit->SetText(supplies.String());
	fAddSuppliesEdit->SetText("0");
	
	if(product->name == ""){
		fNameEdit->MakeFocus();
	}
	else{
		fAddSuppliesEdit->MakeFocus();
	}
	
	fInvalidRequestInvoker = new BInvoker(new BMessage(INVALID_PRODUCT), this);
	
}


void 
EditProductWindow::MessageReceived(BMessage *msg)
{
	BFont font;
	font.SetSize(FONT_SIZE);
	BAlert *a = NULL;
	product_f productinfo;
	BTextControl *textControl;
	switch(msg->what) {
		case EDIT_PRODUCT:
			//save product 
			if(GetProductInfo(&productinfo) == B_OK){
				BMessage reply;
				BMessage msg(SAVE_PRODUCT);
				productinfo.Archive(&msg);
				if(be_app_messenger.SendMessage(&msg,&reply) != B_OK){
					//Quit();
				}
				if(fSell){				
					BMessage sellMsg(SELL_PRODUCT);
					sellMsg.AddString("barcode",productinfo.barcode);
					sellMsg.AddInt32("tradetime",fTradeTime);
					be_app->PostMessage(&sellMsg);
				}
				Quit();
			}
			else{
				a = new BAlert("error",
					"Einige Felder sind nicht korrekt gefüllt.",
					"ok");
				a->TextView()->SetFont(&font);
				a->Go();
			}
			break;
			
		case INVALID_REQUEST:
			a = new BAlert("Produkt Entfernen",
				"Soll das Produkt bis auf weiters aus der Liste entfernt werden?.",
				"Abbrechen", "Ok");
			a->SetFeel(B_FLOATING_APP_WINDOW_FEEL);
			a->TextView()->SetFont(&font);
			a->SetShortcut(0, B_ESCAPE);
			a->SetShortcut(1, B_SPACE);
			a->Go(fInvalidRequestInvoker);
			break;
			
		case INVALID_PRODUCT:
			int32 button;
			msg->FindInt32("which",&button);
			
			if(fSell || button != 1)
				break;
			
			//unvalid product
			if (GetProductInfo(&productinfo) == B_OK) {
				BMessage reply;
				BMessage msg(UNVALID_PRODUCT);
				productinfo.Archive(&msg);
				if (be_app_messenger.SendMessage(&msg,&reply) != B_OK)
					Quit();
				
				Quit();
			}
			else{
				a = new BAlert("error",
					"Einige Felder sind nicht korrekt gefüllt.",
					"ok");
				a->TextView()->SetFont(&font);
				a->Go();
			}
			
			break;
		case ESCAPE_PRESSED:
			Quit();
			break;
		case NUMBER_MODIFIED:
			//parse text of a BTextControl
			msg->FindPointer("source", (void**)&textControl);
			
			ParseNumberTextControl(textControl);
			break;
		default:
	 		BWindow::MessageReceived(msg);
	}
	
}


void	
EditProductWindow::ParseNumberTextControl(BTextControl *textControl)
{
	BString content = textControl->Text();
	for (int32 i = 0; ;i++) {
		char c = content.ByteAt(i);
		if (!c)
			break;

		PRINT(("char %i \n",c));
		if((c < 48 || c > 57) && c != 44 && c != 46 && c != '-'){
			PRINT(("bad char %i \n",c));
			content.Remove(i,1);
			textControl->SetText(content.String());
			textControl->TextView()->Select(i,i);
		}
		else if(c == 44){
			PRINT(("comma char %i \n",c));
			content.Remove(i,1);
			content.Insert(".",i);
			textControl->SetText(content.String());
			textControl->TextView()->Select(i + 1, i + 1);
		}
	}
	//textControl->SetText("moep");
}

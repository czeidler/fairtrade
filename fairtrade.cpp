/*
 * Copyright 2006-2013, Clemens Zeidler, czeidler@gmx.de
 * Distributed under the terms of the MIT License.              
 */

#include "fairtrade.h"

#include <Roster.h>
#include <Entry.h>
#include <Directory.h>
#include <FindDirectory.h>
#include <Path.h>
#include <image.h>
#include <File.h>

#include <unistd.h>

#define DEBUG 1
#include <Debug.h>


TradeApp::TradeApp(char *signature)
	:
	BApplication(signature)
{
	//read config
	BPath path;
	if (find_directory (B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
		return;
	path.Append ("fairtrade");

	BPath configP(path.Path());
	configP.Append(kConfigFile);

	fConfig.ReadConfig(configP.Path());
	
	BPath dbP;
	if(fConfig.databasePath.FindFirst("/") != 0){
		//relative path
		dbP.SetTo(path.Path());
		dbP.Append(fConfig.databasePath);
	} else
		dbP.SetTo(fConfig.databasePath);
	//create dir if not exist
	BString dbDir(dbP.Path());
	dbDir.Truncate(dbDir.IFindLast("/"));
	
	BDirectory dir;
	dir.CreateDirectory(dbDir.String(), &dir);
	
	//set export path
	fExportPath = fConfig.csvExportPath;
	if(fExportPath.FindFirst("/") != 0){
		//relative path
		fExportPath = path.Path();
		fExportPath+= fConfig.csvExportPath;
	}
	
	HideCursor();
	fStock = new Stock(dbP.Path());
	
	fMainWindow = new MainWindow(BRect(100, 300, 400, 700), "Fair Trade", fConfig);
	fMainWindowMessenger = new BMessenger(fMainWindow);
	
	fProducts = new BList;
	fTrades = new BList;
	fBaskets = new BList;
	fMainWindow->fSellView->SetBasketsList(fBaskets);
	fSellNewProduct = false;
	
	fMainWindow->Show();
	
	//read data:
	product_f *prod;
	fStock->GetProducts(fProducts);
	BMessenger messenger(fMainWindow);
	for (int k = 0; ; k++) {
		prod = (product_f*)fProducts->ItemAt(k);
		if (prod == NULL)
			break;

		BMessage productmsg(ADD_PRODUCT);
		productmsg.AddPointer("product", prod);
		messenger.SendMessage(&productmsg);
	}
	
	fStock->GetTrades(fTrades);
	trade_f *trade;
	for (int k = 0; ; k++) {
		trade = (trade_f*)fTrades->ItemAt(k);
		if (trade == NULL)
			break;
			
		product_f *product = FindProduct(trade->productid);
		if (product != NULL) {
			basket_f *basket = FindBasket(trade->date);
			if (basket == NULL) {
				basket = new basket_f(trade->date);
				basket->saved = true;
				fBaskets->AddItem(basket);
			}
			basket->sum+= trade->prize * trade->number;
			trade_info *tradeinfo = new trade_info;
			tradeinfo->trade = trade;
			tradeinfo->product = product;
			tradeinfo->basket = basket;
			tradeinfo->oldnumber = trade->number;
			basket->trades.AddItem(tradeinfo);
		}
	}	
}


TradeApp::~TradeApp()
{
	//cleaning
	delete fMainWindowMessenger;
	delete fStock;
	product_f *prod;
	for (int k = 0; k < fProducts->CountItems(); k++) {
		prod = (product_f*)fProducts->ItemAt(k);
		delete prod;
	}
	delete fProducts;
	
	trade_f *trade;
	for (int k = 0; k < fTrades->CountItems(); k++) {
		trade = (trade_f*)fTrades->ItemAt(k);
		delete trade;
	}
	delete fTrades;
	
	basket_f *basket;
	for (int k = 0; k < fBaskets->CountItems(); k++) {
		basket = (basket_f*)fBaskets->ItemAt(k);
		delete basket;
	}
	delete fBaskets;
}


void 
TradeApp::MessageReceived(BMessage *msg)
{
	BDirectory dir;
	BList list;
	product_f product;
	BString barcode;
	int32 tradetime;
	basket_f *basket;
	int8 number;
	trade_info *tradeInfo;
	int32 arg_c;
	char **arg_v;
	int32 return_value;
	thread_id exec_thread;
	switch (msg->what) {
		case UNVALID_PRODUCT:
			product.Instantiate(msg);
			product.valid = false;
			InsertProductToDatabase(&product);
			break;
		case SAVE_PRODUCT:
			product.Instantiate(msg);
			InsertProductToDatabase(&product);
			break;
		case SELL_PRODUCT:
			msg->FindString("barcode",&barcode);
			msg->FindInt32("tradetime",&tradetime);
			SellProduct(barcode,tradetime);
			break;
		case SAVE_TRADE:
			msg->FindPointer("basket", (void**)&basket);
			if (basket != NULL)
				SaveBasket(basket);
			break;
		case UPDATE_TRADE:
			msg->FindInt8("number",&number);
			msg->FindPointer("tradeinfo", (void**)&tradeInfo);
			UpdateTrade(tradeInfo, number);
			break;
		case REMOVE_TRADE:
			msg->FindPointer("tradeinfo", (void**)&tradeInfo);
			RemoveTrade(tradeInfo);
			break;
		case RESET_PRODUCTS_COUNT:
			PRINT(("ResetProductsCount\n"));
			fStock->ResetProductsCount();
			for (int i = 0; i < fProducts->CountItems(); i++) {
				product_f* product = (product_f*)fProducts->ItemAt(i);
				product->supplies = 0;
				BMessage msg(UPDATE_PRODUCT);
				msg.AddPointer("product", product);
				fMainWindowMessenger->SendMessage(&msg);
			}
			break;
		case QUIT:
			//create the export dir if not exist
			dir.CreateDirectory(fExportPath.String(), &dir);
			ExportTable();
			if(fConfig.shutdownOnExit){
				//shutdown
				arg_c = 1;
				arg_v = (char**)malloc(sizeof(char*) * (arg_c + 1));
				BPath bin;
				find_directory(B_SYSTEM_BIN_DIRECTORY, &bin);
				bin.Append("shutdown");
				arg_v[0] = strdup(bin.Path());
				arg_v[1] = NULL;
				exec_thread = load_image(arg_c, (const char**)arg_v, (const char**)environ);
				free(arg_v);
				wait_for_thread(exec_thread, &return_value);
				PRINT(("shutdown\n"));
			}
			be_app->PostMessage(B_QUIT_REQUESTED);
			break;
		default:
			BApplication::MessageReceived(msg);
	}
}


void			
TradeApp::InsertProductToDatabase(product_f *product)
{
	product_f *prod = FindProduct(product->barcode);
	if (prod != NULL ) {
		fStock->UpdateProduct(prod->id, *product);
		BMessage msg;
		product->id = prod->id;
		product->Archive(&msg);
		UpdateProduct(&msg);
	} else {
		time(&(product->insertdate));
		int32 productId = fStock->AddProduct(*product);
		product->id = productId;
		BMessage msg;
		product->Archive(&msg);
		AddProduct(&msg);
	}
}


void			
TradeApp::AddProduct(BMessage *archive)
{
	product_f *product = new product_f;
	product->Instantiate(archive);
	fProducts->AddItem(product);
	BMessage msg(ADD_PRODUCT);
	msg.AddPointer("product",product);
	fMainWindowMessenger->SendMessage(&msg);
}


void			
TradeApp::UpdateProduct(BMessage *archive)
{
	product_f product;
	product.Instantiate(archive);
	
	product_f *prod = FindProduct(product.barcode);
	if (prod != NULL)
		prod->Instantiate(archive);

	BMessage msg(UPDATE_PRODUCT);
	msg.AddPointer("product", prod);
	fMainWindowMessenger->SendMessage(&msg);
}


void			
TradeApp::SellProduct(BString barcode,time_t date)
{
	product_f *prod = FindProduct(barcode);
	if (prod == NULL) {
		product_f product;
		product.barcode = barcode;
		product.prize = 0;
		BWindow* editWindow = new EditProductWindow(BRect(150,200,700,700),
			"edit product", &product, true,
			fMainWindow->fSellView->InitTradeTime());
		editWindow->Show();
		return;
	}
	//check if not valid
	if(!prod->valid){
		prod->valid = true;
		InsertProductToDatabase(prod);
	}
	
	basket_f *basket = FindBasket(date);
	if (basket == NULL) {
		basket = new basket_f(date);
		fBaskets->AddItem(basket);
	}
		
	trade_info *oldTradeInfo = FindTrade(prod->id,basket);
	if (oldTradeInfo == NULL) {
		basket->sum+= prod->prize;
		
		trade_f *trade = new trade_f;
		trade->productid = prod->id;
		trade->prize = prod->prize;
		trade->date = date;
		trade->number = 1;
		fTrades->AddItem(trade);
		
		trade_info *tradeinfo = new trade_info;
		tradeinfo->trade = trade;
		tradeinfo->product = prod;
		tradeinfo->basket = basket;
		tradeinfo->state = TRADE_STATE_ADD;
		basket->trades.AddItem(tradeinfo);
		
		BMessage msg(ADD_TRADE);
		msg.AddPointer("tradeinfo",tradeinfo);
		tradeinfo->basket->saved = false;
		fMainWindowMessenger->SendMessage(&msg);
	} else if(oldTradeInfo->state == TRADE_STATE_REMOVE) {
		oldTradeInfo->state = TRADE_STATE_UPDATE;
		oldTradeInfo->trade->number = 1;
		BMessage msg(ADD_TRADE);
		msg.AddPointer("tradeinfo",oldTradeInfo);
		basket->sum+= prod->prize;
		oldTradeInfo->basket->saved = false;
		fMainWindowMessenger->SendMessage(&msg);
	} else {
		oldTradeInfo->trade->number+= 1;
		BMessage msg(UPDATE_TRADE);
		msg.AddPointer("tradeinfo", oldTradeInfo);
		basket->sum+= prod->prize;
		oldTradeInfo->basket->saved = false;
		fMainWindowMessenger->SendMessage(&msg);
	}
}


void			
TradeApp::UpdateTrade(trade_info *tradeInfo, int32 number)
{
	float prize = tradeInfo->trade->prize;
	tradeInfo->basket->sum-=prize * tradeInfo->trade->number;
	tradeInfo->basket->sum+=prize * number;
	tradeInfo->trade->number = number;
	
	if (tradeInfo->state != TRADE_STATE_ADD)
		tradeInfo->state = TRADE_STATE_UPDATE;

	tradeInfo->basket->saved = false;
	
	BMessage msg(UPDATE_TRADE);
	msg.AddPointer("tradeinfo", tradeInfo);
	fMainWindowMessenger->SendMessage(&msg);
}


void			
TradeApp::RemoveTrade(trade_info *tradeInfo)
{
	tradeInfo->state = TRADE_STATE_REMOVE;
	tradeInfo->basket->sum-= tradeInfo->trade->prize * tradeInfo->trade->number;
	tradeInfo->basket->saved = false;
	tradeInfo->trade->number = 0;
	BMessage msg(UPDATE_BASKET);
	fMainWindowMessenger->SendMessage(&msg);
}


product_f*	
TradeApp::FindProduct(int32 id)
{
	product_f *prod;
	for (int k = 0; ; k++) {
		prod = (product_f*)fProducts->ItemAt(k);
		if (prod == NULL)
			break;
		
		if (prod->id == id)
			return prod;

	}
	return NULL;
}


trade_info*		
TradeApp::FindTrade(int32 productId, basket_f *basket)
{
	trade_info *tradeInfo;
	for (int k = 0; ; k++) {
		tradeInfo = (trade_info*)basket->trades.ItemAt(k);
		if (tradeInfo == NULL)
			break;
		
		if (tradeInfo->trade->productid == productId)
			return tradeInfo;
	}
	return NULL;
}


product_f*		
TradeApp::FindProduct(BString barcode)
{
	product_f *prod;
	for (int k = 0; ; k++) {
		prod = (product_f*)fProducts->ItemAt(k);
		if (prod == NULL)
			break;

		if (prod->barcode == barcode)
			return prod;
	}
	return NULL;
}


basket_f*		
TradeApp::FindBasket(time_t date)
{
	basket_f *basket;
	for (int k = 0; ; k++) {
		basket = (basket_f*)fBaskets->ItemAt(k);
		if (basket == NULL)
			break;
		
		if (basket->date == date)
			return basket;
	}
	return NULL;
}


void			
TradeApp::SaveBasket(basket_f *basket)
{
	trade_info *info;
	for (int k = 0; ; k++) {
		info = (trade_info*)basket->trades.ItemAt(k);
		if (info == NULL)
			break;

		product_f* product = info->product;
		int32 deltaNumber;
		switch (info->state) {
			case TRADE_STATE_ADD:
				info->trade->id = fStock->AddTrade(*(info->trade));
				info->state = TRADE_STATE_NONE;
				//update the product supplies
				info->product->supplies-= info->trade->number;
				info->oldnumber = info->trade->number;
				fStock->UpdateProduct(info->trade->productid, *(info->product));
				break;
			case TRADE_STATE_UPDATE:
				fStock->UpdateTrade(info->trade->id,*(info->trade));
				deltaNumber = info->trade->number - info->oldnumber;
				info->product->supplies-= deltaNumber;
				info->oldnumber = info->trade->number;
				fStock->UpdateProduct(info->trade->productid, *(info->product));
				info->state = TRADE_STATE_NONE;
				break;
			case TRADE_STATE_REMOVE:
				fStock->RemoveTrade(info->trade->id);
				fTrades->RemoveItem(info->trade);
				deltaNumber = info->trade->number - info->oldnumber;
				info->product->supplies-= deltaNumber;
				fStock->UpdateProduct(info->trade->productid, *(info->product));
				delete info->trade;
				basket->trades.RemoveItem(info);
				delete info;
				break;
		}
		
		basket->saved = true;
		
		//update product list
		BMessage msgUpdate(UPDATE_PRODUCT);
		msgUpdate.AddPointer("product", product);
		BMessage reply;
		fMainWindowMessenger->SendMessage(&msgUpdate, &reply);
	}
	
	if (basket->trades.CountItems() == 0){
		fBaskets->RemoveItem(basket);
		delete basket;
	}
}


void			
TradeApp::ExportTable()
{
	
	time_t timeNow;
	time(&timeNow);
	struct tm *structNow = localtime(&timeNow);
	//printf("%d.%d.%d\n", structNow->tm_mday, structNow->tm_mon + 1, structNow->tm_year + 1900);
	BString dateString = "";
	dateString << structNow->tm_mday; dateString+= "_";
	dateString << structNow->tm_mon + 1; dateString+= "_";
	dateString << structNow->tm_year + 1900; dateString+= "_";
	dateString << structNow->tm_hour; dateString+= "-";
	dateString << structNow->tm_min; 
	
	//export products
	BString productsFileName = fExportPath;
	productsFileName+= "/Produkte_";
	productsFileName+= dateString;
	productsFileName+= ".cvs";
	BFile productFile(productsFileName.String(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
	BString lableString = "Barcode\tName\tKommentar\tPreis\tBestand\tEingefuegt\n";
	productFile.Write(lableString.String(),lableString.Length());
	product_f *prod;
	for (int k = 0; ; k++) {
		prod = (product_f*)fProducts->ItemAt(k);
		if (prod == NULL)
			break;
		
		if (!prod->valid)
			continue;
		
		BString outString = "";
		outString << prod->barcode; outString+= "\t";
		outString << prod->name; outString+= "\t";
		outString << prod->comment; outString+= "\t";
		outString << prod->prize; outString+= "\t";
		outString << prod->supplies; outString+= "\t";
		
		struct tm *structInsertDate = localtime(&(prod->insertdate));
		outString << structInsertDate->tm_mday; outString+= ".";
		outString << structInsertDate->tm_mon + 1; outString+= ".";
		outString << structInsertDate->tm_year + 1900; outString+= " ";
		outString << structInsertDate->tm_hour; outString+= ":";
		outString << structInsertDate->tm_min; 
	
		outString+= "\n";
		productFile.Write(outString.String(),outString.Length());
	}
	
	//trade list
	BString tradeSimpleFileName = fExportPath;
	tradeSimpleFileName+= "/Verkaeufe_";
	tradeSimpleFileName+= dateString;
	tradeSimpleFileName+= ".cvs";
	BFile tradeSimpleFile(tradeSimpleFileName.String(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
	lableString = "Tag\tSumme\n";
	tradeSimpleFile.Write(lableString.String(),lableString.Length());
	
	BString tradeDetailFileName = fExportPath;
	tradeDetailFileName+= "/Verkaeufe_Detailiert_";
	tradeDetailFileName+= dateString;
	tradeDetailFileName+= ".cvs";
	BFile tradeDetailFile(tradeDetailFileName.String(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
	lableString = "Datum\tProdukt\tBarcode\tVerkaufspreis\tAnzahl\n";
	tradeDetailFile.Write(lableString.String(),lableString.Length());
	
	basket_f *basket;
	//neaded for simple list:
	float daySum = 0;
	int32 ydayOfTrade = -1;
	int32 yearOfTrade = -1;
	int32 dayOfTrade = -1;
	int32 monthOfTrade = -1;
	BString outSimpleString = "";
	struct tm *time;
	for (int k = 0; ; k++) {
		basket = (basket_f*)fBaskets->ItemAt(k);
		if (basket == NULL) {
			if(dayOfTrade > 0){
				outSimpleString = "";
				outSimpleString << time->tm_mday; outSimpleString+= ".";
				outSimpleString << time->tm_mon + 1; outSimpleString+= ".";
				outSimpleString << time->tm_year + 1900; outSimpleString+= "\t";
				outSimpleString << daySum; outSimpleString+= "\n";
				tradeSimpleFile.Write(outSimpleString.String(),outSimpleString.Length());
			}
			break;
		}
		if (basket->saved) {
			time = localtime(&(basket->date));
			//simple trade list
			if (k == 0) {
				ydayOfTrade = time->tm_yday;
				dayOfTrade = time->tm_mday;
				monthOfTrade = time->tm_mon;
				yearOfTrade = time->tm_year;
			}
			if (time->tm_yday == ydayOfTrade && time->tm_year == yearOfTrade)
				daySum+= basket->sum;
			else {
				outSimpleString = "";
				outSimpleString << dayOfTrade; outSimpleString+= ".";
				outSimpleString << monthOfTrade + 1; outSimpleString+= ".";
				outSimpleString << yearOfTrade + 1900; outSimpleString+= "\t";
				outSimpleString << daySum;  outSimpleString+= "\n";
				tradeSimpleFile.Write(outSimpleString.String(),outSimpleString.Length());
				ydayOfTrade = time->tm_yday;
				dayOfTrade = time->tm_mday;
				monthOfTrade = time->tm_mon;
				yearOfTrade = time->tm_year;
				daySum = basket->sum;
			}
			//detail trade list
			BString outDetailString = "";
			trade_info *tradeInfo;
			for (int k = 0; ; k++) {
				tradeInfo = (trade_info*)basket->trades.ItemAt(k);
				if (tradeInfo == NULL)
					break;
				
				outDetailString << time->tm_mday; outDetailString+= ".";
				outDetailString << time->tm_mon + 1; outDetailString+= ".";
				outDetailString << time->tm_year + 1900; outDetailString+= " ";
				outDetailString << time->tm_hour; outDetailString+= ":";
				outDetailString << time->tm_min; outDetailString+= "\t";
				outDetailString << tradeInfo->product->name; outDetailString+= "\t";
				outDetailString << tradeInfo->product->barcode; outDetailString+= "\t";
				outDetailString << tradeInfo->trade->prize; outDetailString+= "\t";
				outDetailString << tradeInfo->trade->number; outDetailString+= "\n";				
			}
			outDetailString+= "Summe\t";
			outDetailString << basket->sum;
			outDetailString+="\n\n";
			tradeDetailFile.Write(outDetailString.String(),outDetailString.Length());
		}
	}
}


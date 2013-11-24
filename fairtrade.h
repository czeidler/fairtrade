/*
 * Copyright 2006-2013, Clemens Zeidler, czeidler@gmx.de
 * Distributed under the terms of the MIT License.              
 */

#ifndef FAIRTRADE_H
#define FAIRTRADE_H

#include <Application.h>

#include "ConfigFairTrade.h"
#include "mainwindow.h"
#include "productsview.h"
#include "stock.h"


class TradeApp  : public BApplication {

	public:
						TradeApp(char *signature);
						~TradeApp();
		virtual	void	MessageReceived(BMessage *msg);
	private:
		void			InsertProductToDatabase(product_f *product);
		void			AddProduct(BMessage *archive);
		void			UpdateProduct(BMessage *archive);
		void			SellProduct(BString barcode, time_t date);
		void			UpdateTrade(trade_info *tradeInfo, int32 number);
		void			RemoveTrade(trade_info *tradeInfo);
		void			ExportTable();
		
		product_f*		FindProduct(int32 productId);  	//returns NULL if nu product is found
		trade_info*		FindTrade(int32 productId, basket_f *basket);	  	//returns NULL if nu product is found
		product_f*		FindProduct(BString barcode);  	//returns NULL if nu product is found
		basket_f*		FindBasket(time_t date);		//returns NULL if nu basket is found
		void			SaveBasket(basket_f *basket);
		MainWindow *fMainWindow;
		BMessenger *fMainWindowMessenger;	
		Stock *fStock;  
		BList	*fProducts;
		BList	*fTrades;
		BList	*fBaskets;
		bool fSellNewProduct;
		config fConfig;
		BString fExportPath;
};


#endif

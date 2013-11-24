/*
 * Copyright 2006-2013, Clemens Zeidler, czeidler@gmx.de
 * Distributed under the terms of the MIT License.              
 */

#ifndef STOCK_H
#define STOCK_H

#include <String.h>
#include <List.h>
#include <Locker.h>
#include <Messenger.h>

#include "SQLConnection.h"

const uint TRADE_STATE_NONE = 0;
const uint TRADE_STATE_ADD = 1;
const uint TRADE_STATE_UPDATE = 2;
const uint TRADE_STATE_REMOVE = 3;


typedef struct product_f
{
	product_f();
	void Archive(BMessage *archive);
	void Instantiate(BMessage *archive);
	int32 id;
	BString name;
	BString barcode;
	BString comment;
	float prize;
	int32 supplies;
	bool valid;			//set to false when product is removed
	time_t insertdate;
};


typedef struct trade_f
{
	int32 id;
	int32 productid;
	float prize;
	time_t date;
	uint32 number;		//number of sells
};


typedef struct basket_f
{
	basket_f(time_t date);
	~basket_f();
	time_t date;
	BList trades;
	float sum;	
	bool saved;	
};


typedef struct trade_info
{
	trade_info();
	trade_f *trade;
	product_f *product;	
	basket_f *basket;
	uint8 state;
	int32 oldnumber;	
};


class Stock
{
	public:
		Stock(BString filename);
		~Stock();
		
		int32 AddProduct(product_f &product); //if product already in the list the product will be updated
		int32 AddTrade(trade_f &trade);		 //if trade already in the list the trade will be updated
		void UpdateTrade(uint id, trade_f &trade);
		void UpdateProduct(uint id, product_f &product);
		void RemoveTrade(uint id);
		status_t GetProducts(BList* list);
		status_t GetTrades(BList* list);
		
	private:
		SQLiteConnection *fDatabase;
		BLocker fLock;
};


#endif

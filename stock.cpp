/*
 * Copyright 2006-2013, Clemens Zeidler, czeidler@gmx.de
 * Distributed under the terms of the MIT License.              
 */

#include "stock.h"

#include <Message.h>

#define DEBUG 1
#include <Debug.h>


basket_f::basket_f(time_t d)
{
	date = d;
	sum = 0;
	saved = false;
}


basket_f::~basket_f()
{
	trade_info* trade;
	for (int k = 0; ; k++) {
		trade = (trade_info*)trades.ItemAt(k);
		if (trade == NULL)
			break;
		delete trade;
	}
}


trade_info::trade_info()
{
	state = TRADE_STATE_NONE;
}


product_f::product_f()
{
	barcode = "";
	prize = 0;
	supplies = 0;
	valid = true;
}


void 
product_f::Archive(BMessage *msg)
{
	msg->AddInt32("id",id);
	msg->AddString("name",name);
	msg->AddString("barcode",barcode);
	msg->AddString("comment",comment);
	msg->AddFloat("prize",prize);
	msg->AddInt32("supplies",supplies);
	msg->AddBool("valid",valid);
	msg->AddInt32("insertdate",insertdate);
}


void 
product_f::Instantiate(BMessage *msg)
{
	msg->FindInt32("id",&id);
	msg->FindString("name",&name);
	msg->FindString("barcode",&barcode);
	msg->FindString("comment",&comment);
	msg->FindFloat("prize",&prize);
	msg->FindInt32("supplies",&supplies);
	msg->FindBool("valid",&valid);
	msg->FindInt32("insertdate",&insertdate);
}


Stock::Stock(BString filename)
{
	fDatabase = new SQLiteConnection(filename.String());
	// set up tables. This shouldn't harm already existing tables.
	// products:
	fDatabase->Exec("CREATE TABLE products (id INTEGER PRIMARY KEY, name TEXT, barcode TEXT, comment TEXT, prize FLOAT, supplies INTEGER, valid INTEGER, insertdate DATE )");
	fDatabase->Exec("CREATE INDEX products_id_index ON products (id)");
	fDatabase->Exec("CREATE INDEX products_name_index ON products (name)");
	fDatabase->Exec("CREATE INDEX products_barcode_index ON products (barcode)");
	fDatabase->Exec("CREATE INDEX products_comment_index ON products (comment)");
	fDatabase->Exec("CREATE INDEX products_prize_index ON products (prize)");
	fDatabase->Exec("CREATE INDEX products_supplies_index ON products (supplies)");
	fDatabase->Exec("CREATE INDEX products_valid_index ON products (valid)");
	fDatabase->Exec("CREATE INDEX products_insertdate_index ON products (insertdate)");
	
	// trades:
	fDatabase->Exec("CREATE TABLE trades (id integer primary key, productid integer, prize float, date datetime, number integer)");
	fDatabase->Exec("CREATE INDEX trades_id_index ON trades (id)");
	fDatabase->Exec("CREATE INDEX trades_productid_index ON trades (productid)");
	fDatabase->Exec("CREATE INDEX trades_prize_index ON trades (prize)");
	fDatabase->Exec("CREATE INDEX trades_date_index ON trades (date)");
	fDatabase->Exec("CREATE INDEX trades_number_index ON trades (number)");
}


Stock::~Stock()
{
	delete fDatabase;
}


int32
Stock::AddProduct(product_f &product)
{
	fLock.Lock();
	BString query = "INSERT INTO products (name, barcode, comment, prize, supplies, valid, insertdate) VALUES ('";
	query += product.name; query += "','";
	query += product.barcode; query += "','";
	query += product.comment; query += "','";
	query << product.prize; query += "','";
	query << product.supplies; query += "','";
	if (product.valid)
		query << "1','";
	else
		query << "0','";
	query << product.insertdate; 
	query += "');";
	PRINT(("query string %s\n", query.String() ));
	fDatabase->Exec(query.String());
	fDatabase->Exec("COMMIT");
	
	query = "SELECT id FROM products WHERE barcode='";
	query += product.barcode;
	query += "';";
	fDatabase->Exec(query.String());
	int32 productId = -1;
	if (fDatabase->NumRows() >= 0)
		productId = atoi(fDatabase->GetValue(0,"id").c_str());

	fLock.Unlock();

	return productId;
}


void 
Stock::UpdateProduct(uint id, product_f &product)
{
	fLock.Lock();
	BString query = "UPDATE products ";
	query += "SET name='";  query += product.name; query +="' ";
	query += ", barcode='"; query += product.barcode; query +="' ";
	query += ", comment='"; query += product.comment; query +="' ";
	query += ", prize='"; query << product.prize; query +="' ";
	query += ", supplies='"; query << product.supplies; query +="' ";
	query += ", valid='"; query << product.valid; query +="' ";
	query += ", insertdate='"; query << product.insertdate; query +="' ";
	query += "WHERE id="; query << id;
	fDatabase->Exec(query.String());
	fDatabase->Exec("COMMIT");
	fLock.Unlock();
}


int32 
Stock::AddTrade(trade_f &trade)
{
	fLock.Lock();
	BString query = "INSERT INTO trades (productid, prize, date, number) VALUES ('";
	query << trade.productid; query += "','";
	query << trade.prize; query += "','";
	query << trade.date; query += "','";
	query << trade.number;
	query += "');";
	fDatabase->Exec(query.String());
	fDatabase->Exec("COMMIT");
	
	query = "SELECT id FROM trades WHERE productid='";
	query << trade.productid;
	query += "';";
	fDatabase->Exec(query.String());
	int32 tradeId = -1;
	if(fDatabase->NumRows() >= 0){
		tradeId = atoi(fDatabase->GetValue(0,"id").c_str());
	}
	PRINT(("new trade id %i\n", tradeId));
	
	fLock.Unlock();
	return tradeId;
}


void 
Stock::UpdateTrade(uint id, trade_f &trade)
{
	fLock.Lock();
	BString query = "UPDATE trades ";
	query += "SET productid='"; query << trade.productid; query +="' ";
	query += ", prize='"; query << trade.prize; query +="' ";
	query += ", date='"; query << trade.date; query +="' ";
	query += ", number='"; query << trade.number; query +="' ";
	query += "WHERE id="; query << id;
	fDatabase->Exec(query.String());
	fDatabase->Exec("COMMIT");
	fLock.Unlock();
}


void 
Stock::RemoveTrade(uint id)
{
	fLock.Lock();
	BString query = "DELETE FROM trades WHERE id=";
	query << id;
	fDatabase->Exec(query.String());
	fDatabase->Exec("COMMIT");
	fLock.Unlock();
}


status_t 
Stock::GetProducts(BList* list)
{
	fLock.Lock();
	BString query = "SELECT * FROM products";
	fDatabase->Exec(query.String());
	for (int i = 0; i < fDatabase->NumRows(); i++) {
		product_f *product = new product_f;
		list->AddItem(product);
		
		product->id  = atoi(fDatabase->GetValue(i,0).c_str());
		product->name = fDatabase->GetValue(i,1).c_str(); 
		product->barcode = fDatabase->GetValue(i,2).c_str();
		product->comment = fDatabase->GetValue(i,3).c_str(); 
		product->prize = atof(fDatabase->GetValue(i,4).c_str());
		product->supplies = atoi(fDatabase->GetValue(i,5).c_str());
		product->valid = atoi(fDatabase->GetValue(i,6).c_str());
		product->insertdate = atoi(fDatabase->GetValue(i,7).c_str());
		//PRINT(("insertdate %i\n", product->insertdate));
	}
	fLock.Unlock();
	return B_OK;
}


status_t 
Stock::GetTrades(BList* list)
{
	fLock.Lock();
	BString query = "SELECT * FROM trades";
	fDatabase->Exec(query.String());
	BString result;
	for (int i=0; i<fDatabase->NumRows(); i++ )
	{
		trade_f *trade = new trade_f;
		list->AddItem(trade);
		trade->id = atoi(fDatabase->GetValue(i,0).c_str());
		trade->productid = atoi(fDatabase->GetValue(i,1).c_str());
		trade->prize = atof(fDatabase->GetValue(i,2).c_str());
		trade->date = atoi(fDatabase->GetValue(i,3).c_str());
		trade->number = atoi(fDatabase->GetValue(i,4).c_str());
	}
	fLock.Unlock();
	return B_OK;
}


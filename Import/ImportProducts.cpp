/*
 * Copyright 2006-2013, Clemens Zeidler, czeidler@gmx.de
 * Distributed under the terms of the MIT License.              
 */
 
#include "ImportProducts.h"

#include <fstream>
#include <sstream>

#define DEBUG 1
#include <Debug.h>



ImportProducts::ImportProducts(const char* input, const char* databasePath)
{
	fInputPath = input;
	fDatabase = new Stock(databasePath);
}


ImportProducts::~ImportProducts()
{
	delete fDatabase;
}


void	
ImportProducts::Import()
{
	ifstream stream(fInputPath);
	
	while (true && stream){
		if (stream.eof() ){
			break;
		}	
		product_f product;
		char buffer[1023];
		stream.getline(buffer, 1024);
		stringstream line(buffer);
		//line.get(buffer, 1024, '\t');
		
		//barcode
		char buf[256];
		line.getline(buf, 256, '\t');
		string word = buf;
		if(word == "Barcode" || word == ""){
			continue;
		}
		product.barcode = word.c_str();
		//name
		line.getline(buf, 256, '\t');
		product.name = buf;
		//comment
		line.getline(buf, 256, '\t');
		product.comment = buf;
		//prize
		double prize;
		line >> prize;
		product.prize = prize;
		//supplies
		int32 supplies;
		line >> supplies;
		product.supplies = supplies;
		//valid
		product.valid = true;
		//insert time
		time_t timeNow;
		time(&timeNow);
		product.insertdate = timeNow;
		
		fDatabase->AddProduct(product);
						
	}
	
}




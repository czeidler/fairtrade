/*
 * Copyright 2006-2013, Clemens Zeidler, czeidler@gmx.de
 * Distributed under the terms of the MIT License.              
 */

#include "ImportProducts.h"


int 
main(int argc, char* argv[])
{
	if(argc < 3){
		cout << "usage: ImportProduct input outputDatabase" << endl;
		return -1;
	}
	const char* inputFile = argv[1];
	const char* database = argv[2];
	
	ImportProducts importer(inputFile, database);
	importer.Import();
	return 0;
}

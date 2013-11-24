/*
 * Copyright 2006-2013, Clemens Zeidler, czeidler@gmx.de
 * Distributed under the terms of the MIT License.              
 */

#ifndef IMPORTPRODUCTS_H
#define IMPORTPRODUCTS_H

#include "stock.h"


class ImportProducts {
public:
								ImportProducts(const char* input,
									const char* databasePath);
	virtual						~ImportProducts();
			void				Import();
				
private:
			const char*			fInputPath;
			Stock*				fDatabase;
};


#endif

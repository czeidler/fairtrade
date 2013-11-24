/*
 * Copyright 2006-2013, Clemens Zeidler, czeidler@gmx.de
 * Distributed under the terms of the MIT License.              
 */

#ifndef PRODUCTSVIEW_H
#define PRODUCTSVIEW_H

#include <Button.h>
#include <MessageFilter.h>
#include <TextControl.h>
#include <TextView.h>
#include <Window.h>

#include "ColumnListView.h"
#include "ColumnTypes.h"

#include "stock.h"
#include "blayout.h"

const uint EDIT_PRODUCT = '&edp';
const uint ADD_PRODUCT = '&adp';
const uint SAVE_PRODUCT = '&spr';
const uint UNVALID_PRODUCT = '&unv';
const uint PRODUCT_INVOKED = '&piv';
const uint ADD_TRADE = '&adt';
const uint NUMBER_MODIFIED = '&nmf';


class ProductRow : public BRow
{
public:
			product_f*			product;
};


class ProductView : public BView
{
public:
								ProductView(BRect bounds);
	virtual void				FrameResized(float width, float height);
	virtual void				MakeFocus(bool focused);
		
			void				AddProduct(product_f *product);
				
			void				BarcodeEntered(BString barcode);
			void				EditProduct(product_f *product = NULL);
			void				UpdateProduct(product_f *product);
				
			BColumnListView*	fProductListView;
	
private:
			BColumn*			fNameColumn;
			BColumn*			fPrizeColumn;
			BColumn*			fSuppliesColumn;
			BList				fUnvalidList;
};


class EditProductWindow  : public BWindow 
{
public:
								EditProductWindow(BRect, const char*,product_f *product, bool sell = false, int32 tradeTime = 0);	
	virtual	~EditProductWindow()
	{
		delete fInvalidRequestInvoker;
	}
	
	virtual	void				MessageReceived(BMessage *msg);
		
			status_t			GetProductInfo(product_f *productinfo);
			void				ParseNumberTextControl(BTextControl *textControl);

private:
			int32				fInitialSupplies;
			bool				fSell;
			int32				fTradeTime;
		
			BoxLayout*			fProductLayout;
			BoxLayout*			fHLayout;
		
			BTextControl*		fNameEdit;
			BTextControl*		fBarcodeEdit;
			BTextControl*		fPrizeEdit;
			BTextControl*		fCommentEdit;
		
			BTextControl*		fAddSuppliesEdit;
			BTextControl*		fSuppliesEdit;
		
			BButton*			fOkButton;
		
			BInvoker*			fInvalidRequestInvoker;
};


#endif

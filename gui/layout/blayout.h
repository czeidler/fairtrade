#ifndef BLAYOUT_H
/*
 * Copyright 2006-2013, Clemens Zeidler, czeidler@gmx.de
 * Distributed under the terms of the MIT License.              
 */

#define BLAYOUT_H

#include <View.h>
#include <Rect.h>
#include <List.h>


const int8 HORIZONTAL = 1;
const int8 VERTICAL = 2;


typedef struct view_info
{
	BView* view;
	float weight;
	bool useownwidth;
};


void DivideSame(BList *list); //devide list of textcontrols


class BoxLayout : public BView
{
public:
								BoxLayout(BRect rect, const char *name,
									uint32 resizingMode);
								~BoxLayout();
								
			void 				AttachedToWindow(void);
		
			void 				AddView(BView *view, float weight,
									bool useownwidth = false);
			void				AddSpacer(float weight);
			void				SetAlignment(int8 orientation);
private:
			void				CalculateViews();
			
			BList*				fListOfViews;
			float				fTotalWeight;
			bool				fHorizontal;
			float				fSpace;
};


#endif

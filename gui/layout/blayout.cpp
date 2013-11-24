/*
 * Copyright 2006-2013, Clemens Zeidler, czeidler@gmx.de
 * Distributed under the terms of the MIT License.              
 */

#include "blayout.h"

#include <TextControl.h>

#define DEBUG 1
#include <Debug.h>


void DivideSame(BList *list)
{
	float maxDivider = 0;
	
	//find max divider
	BTextControl *control;
	for ( int k=0; ; k++){
		control = (BTextControl*)list->ItemAt(k);
		if(!control){
			break;
		}
		if(control->Divider() > maxDivider){
			maxDivider = control->Divider();
		}
	}
	
	//set max divider
	for ( int k=0; ; k++){
		control = (BTextControl*)list->ItemAt(k);
		if(!control){
			break;
		}
		control->SetDivider(maxDivider);
	}
}


BoxLayout::BoxLayout(BRect rect, const char *name, uint32 resizingMode)
		:BView(rect, name, resizingMode,B_FRAME_EVENTS)
{
	fListOfViews = new BList;
	fTotalWeight = 0;
	fHorizontal = false;
	fSpace = 0;
}


BoxLayout::~BoxLayout()
{
	view_info* info;
	for ( int k=0; ; k++){
		info = (view_info*)fListOfViews->ItemAt(k);
		if(!info){
			break;
		}
		delete info;
	}
	delete fListOfViews;
}


void 	
BoxLayout::AttachedToWindow()
{
	CalculateViews();
}


void
BoxLayout::AddView(BView *view, float weight, bool useownwidth)
{
	view_info *viewInfo = new view_info;
	viewInfo->view = view;
	viewInfo->weight = weight;
	viewInfo->useownwidth = useownwidth;
	fTotalWeight += weight;
	fListOfViews->AddItem(viewInfo);
	
	CalculateViews();
	if(view){
		AddChild(view);
	}
}


void 
BoxLayout::AddSpacer(float weight)
{
	AddView(NULL, weight);
}


void 
BoxLayout::SetAlignment(int8 orientation)
{
	if(orientation == HORIZONTAL){
		fHorizontal = true;
	}
	else{
		fHorizontal = false;
	}
}


void 
BoxLayout::CalculateViews()
{
	BRect frame = Frame();
	
	float viewPosition = 0;
	float viewWidth;
	float viewHeight;
	float viewLeft;
	float viewTop;
	
	if(Window())
	{
		int32 viewNumber = fListOfViews->CountItems();
		view_info* info;
		for ( int k=0; ; k++){
			info = (view_info*)fListOfViews->ItemAt(k);
			if(!info){
				break;
			}
			
			if(fHorizontal)
			{
				viewWidth = (frame.Width() - fSpace * viewNumber) * info->weight / fTotalWeight;
				if(viewNumber - 1 == k){
					//(round errors)
					viewWidth = ceil(frame.Width() - viewPosition);
				}
				viewHeight = frame.Height();
				viewLeft = viewPosition;
				viewTop = 0;
				
				viewPosition+= viewWidth + fSpace;
			}
			else
			{
				viewWidth = frame.Width();
				viewHeight = (frame.Height() - fSpace * viewNumber) * info->weight / fTotalWeight;
				if(viewNumber - 1 == k){
					//(round errors)
					viewHeight = ceil(frame.Height() - viewPosition);
				}
				viewLeft = 0;
				viewTop = viewPosition;
				
				viewPosition+= viewHeight + fSpace;
			}
			if(info->view){
				if(info->useownwidth){
					viewWidth = info->view->Bounds().Width();
					viewLeft+= ((frame.Width() * info->weight / fTotalWeight) - viewWidth) / 2;
				}
				
				//init the right size
				info->view->ResizeTo(viewWidth, viewHeight);
				
				//set better size:
				float width;
				float height;
				info->view->ResizeToPreferred();
				info->view->GetPreferredSize(&width,&height);
				
				info->view->ResizeTo(viewWidth, height);
				
				//center
				if(fHorizontal){
					viewTop+= (frame.Height()  - height) / 2;
				}
				else{
					viewTop+= ((frame.Height() * info->weight / fTotalWeight) - height) / 2;
				}
				
				info->view->MoveTo(viewLeft, viewTop);
							
			}
	
		}
	}
	
}



/*
 * Copyright 2006-2013, Clemens Zeidler, czeidler@gmx.de
 * Distributed under the terms of the MIT License.              
 */

#ifndef TIMER_H
#define TIMER_H

#include <Invoker.h>

class Timer : public BInvoker
{
	public:
		void Start(bigtime_t sleepTime);
		
	private:
		static int32 TimerThreadFunc(void *arg);
		bigtime_t fTime;
		thread_id fThreadID;
};


#endif

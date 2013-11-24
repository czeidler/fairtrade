/*
 * Copyright 2006-2013, Clemens Zeidler, czeidler@gmx.de
 * Distributed under the terms of the MIT License.              
 */

#include "timer.h"


int32 
Timer::TimerThreadFunc(void *arg)
{
	Timer *timer = (Timer*)arg;
	snooze(timer->fTime);
	timer->Invoke();
	return B_OK;
}


void
Timer::Start(bigtime_t sleepTime)
{
	fTime = sleepTime;
	fThreadID = spawn_thread(TimerThreadFunc, "timer_thread", B_NORMAL_PRIORITY, this);
	resume_thread(fThreadID);
}

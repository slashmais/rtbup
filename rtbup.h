#ifndef _rtbup_rtbup_h_
#define _rtbup_rtbup_h_


#include "monitor.h"
#include "tparm.h"

#include <pthread.h>


struct rtbup
{
	THREAD_PARM TP;
	pthread_t nThreadID;
	int fdInotify;

	rtbup();
	~rtbup();

	bool StartAll();
	void StopAll();
	void RestartMonitors();
};



#endif

#ifndef _rtbup_threadparm_h_
#define _rtbup_threadparm_h_


#include "globs.h"

#include <string>
#include <vector>


struct THREAD_PARM
{
	int fdInotify;
	mapMonData MB;

	THREAD_PARM();

	bool GotEntries();
	bool Excluded(const std::string sF, const std::vector<std::string> &vex);
	void BackItUp(const std::string sM, const std::string sF);
};



#endif

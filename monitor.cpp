
#include "monitor.h"
#include "tparm.h"

#include <unistd.h>
#include <sys/inotify.h>
#include <cerrno>


#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )


volatile bool bStopMonitors;

void* DoMonitor(void* pTP)
{
	THREAD_PARM *ptp = (THREAD_PARM*)pTP;

	if (!ptp->GotEntries()) return NULL;

	//---------------------------------------------------------------------
	std::map<int, std::string> WM;
    std::string mondir;
    std::string sl;
	int wd;

	//---------------------------------------------------------------------
	uint32_t MONITOR_MASK = IN_MODIFY | IN_CREATE | IN_MOVED_TO | IN_DONT_FOLLOW;

	//---------------------------------------------------------------------
	wd = inotify_add_watch(ptp->fdInotify, G_LOG_FILE.c_str(), MONITOR_MASK);
	WM[wd] = G_LOG_FILE;

	//---------------------------------------------------------------------
	for (auto p:ptp->MB)
	{
		mondir = p.first;

		wd = inotify_add_watch(ptp->fdInotify, mondir.c_str(), MONITOR_MASK);

		if (wd != -1)
		{
			WM[wd] = mondir;
			MonitorData *pbx = &p.second;
			sl = cspf(" - Start: %s [backup = %s]", mondir.c_str(), pbx->bupdir.c_str());
			if (pbx->vex.size() > 0)
			{
				std::string sT="";
				for (int i=0; i < (int)pbx->vex.size(); i++)
				{
					if (!sT.empty()) sT.append(", ");
					sT.append(pbx->vex.at(i));
				}
				sl += "\n                     - Excluding: "; //spaces to line up
				sl.append(sT);
			}
			WriteToLog(sl);
			TellUser(sl);
		}
		else
		{
			sl=cspf(" - Error[%d] Cannot create monitor: ", errno);
			switch(errno)
			{
				case EBADF: sl.append("The given file descriptor is not valid."); break;
				case EFAULT: sl.append("pathname points outside of the process's accessible address space."); break;
				case EINVAL: sl.append("The given event mask contains no legal events; or fd is not an inotify file descriptor."); break;
				case ENOMEM: sl.append("Insufficient kernel memory was available."); break;
				case ENOSPC: sl.append("The user limit on inotify watches was reached or the kernel failed to allocate a resource."); break;
				default: sl.append("Unknown error"); break;
			}
			WriteToLog(sl);
			TellUser(sl);
		}
	}

	//---------------------------------------------------------------------
	char EventBuf[BUF_LEN];
	struct inotify_event *pEvt;
	int i, length;

	//watch-loop-----------------------------------------------------------
	while (!bStopMonitors)
	{
		length = read(ptp->fdInotify, EventBuf, BUF_LEN );
		if ( length > 0 )
		{
			i=0;
			while (i < length)
			{
				pEvt = (struct inotify_event*) &EventBuf[i];
				if (pEvt->len)
				{
					if (!scmp(pEvt->name, G_LOG_FILE.c_str())) //to unblock read(..)
					{
						ptp->BackItUp(WM[pEvt->wd], pEvt->name);
					}
				}
				i += EVENT_SIZE + pEvt->len;
			}
		}
	}

	//clean-up the watches-------------------------------------------------
	for (auto p:WM)
	{
		inotify_rm_watch(ptp->fdInotify, p.first); //WMIT->first);
	}

	//---------------------------------------------------------------------

	return NULL;
}


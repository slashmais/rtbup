/*
    This file is part of the rtbup application.

    rtbup - Copyright 2010, 2017 Marius Albertyn

    rtbup is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    rtbup is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with rtbup in a file named COPYING.
    If not, see <http://www.gnu.org/licenses/>.
*/

#include "rtbup.h"
#include "globs.h"

#include <string>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/inotify.h>



rtbup::rtbup() : TP(), nThreadID(0), fdInotify(0) { }
rtbup::~rtbup() { StopAll(); }


bool rtbup::StartAll()
{
	std::string sl="";

	if (fdInotify != 0) StopAll();
	LoadConfigData(TP.MB);
	if (TP.GotEntries())
	{
		fdInotify = inotify_init();
		if (fdInotify != -1)
		{
			bStopMonitors=false;
			TP.fdInotify = fdInotify;
			int r = pthread_create(&nThreadID, NULL, DoMonitor, &TP);
			if (r != 0)
			{
				sl=cspf(" - Error[%d] Cannot create thread: ", r);
				switch(r)
				{
					case EAGAIN: sl.append("Resource limits exceeded"); break;
					case EINVAL: sl.append("Internal error (invalid settings in attr)"); break;
					case EPERM:  sl.append("No Permission"); break;
					default: sl.append("Unknown error"); break;
				}
				WriteToLog(sl);
				TellUser(sl);
				StopAll();
				return false;
			}
			return true;
		}
		else
		{
			sl=cspf(" - Error[%d] Cannot initialize: ", errno);
			switch(errno)
			{
				case EMFILE: sl.append("The user limit on the total number of inotify instances has been reached."); break;
				case ENFILE: sl.append("The system limit on the total number of file descriptors has been reached."); break;
				case ENOMEM: sl.append("Insufficient kernel memory is available."); break;
				default: sl.append("Unknown error"); break;
			}
			WriteToLog(sl);
			TellUser(sl);
			StopAll();
			return false;
		}
	}
	else TellUser(" - Nothing to do -");
	return false;
}

void rtbup::StopAll()
{
	if (nThreadID != 0)
	{
		bStopMonitors=true;
		WriteToLog(" - Stopping monitors ...");
		pthread_join(nThreadID, NULL);
		nThreadID = 0L;
	}
	if (fdInotify != 0)
	{
		close(fdInotify);
		fdInotify = 0;
	}
}

void rtbup::RestartMonitors()
{
	WriteToLog("*** Restarting ***");
	StopAll();
	StartAll();
}



#include "rtbup.h"
#include "globs.h"
#include "rthelp.h"

#include <unistd.h>
#include <iostream>
#include <signal.h>


volatile bool bQuit;
volatile bool bRestart;


void SignalHandler(int sig)
{
	switch(sig)
	{
		case RESTART:				bRestart = true; break;
		case SIGINT: case STOP:		bQuit = true; break;
	}
}


int main(int argc, char *argv[])
{
	std::string s="";
	RTB_Status status;

	//check&get any cmdline args-----------------------------------------------
	if (argc > 1)
	{
		s = argv[1];
		TRIM(s, "- ");
		if (sicmp(s, "help") || sicmp(s, "?")) { ShowHelp(); return 0; }
		//restart/stop background process or just ignore if none
		if (sicmp(s, "restart")) { if (PostRestart()) return 0; }
		else if (sicmp(s, "stop")) { if (PostQuit()) return 0; }
	}

	//initialize globals-------------------------------------------------------
	G_ISATTY = isatty(0);

	//process status & cmdline args--------------------------------------------
	status = GetStatus();
	switch(status)
	{
		case RTB_BEU:	case RTB_BEB:	case RTB_UEU:	{ std::cout << "\nrtbup is already running.\n"; return 0; } break;
		case RTB_UEB:									{ ShowBackgroundHelp(); return 0; } break;
		case RTB_B:		case RTB_U:						{ Startup(); } break;
	}

	//setup signals------------------------------------------------------------
	signal(RESTART, SignalHandler);
	signal(STOP,	SignalHandler);
	signal(SIGINT,	SignalHandler);

	//pre-set loop controls----------------------------------------------------
	bRestart = bQuit = false;

	//main loop----------------------------------------------------------------
	while (!bQuit)
	{
		spf(s, "Started");
		WriteToLog(s);
		TellUser(cspf("\nChange the config-file '%s'\nto add directories to be monitored.\n", G_CONFIG_FILE.c_str()));

		rtbup sbMain;
		if (sbMain.StartAll())
		{
			sleep(1); //give the monitors time to start
			if (G_ISATTY) //ui
			{
				while (!bQuit) //mostly to see TellUser()-output..
				{
					char c=0;
					std::cout << "\nPress q to stop monitoring & quit rtbup";
					while ((c != 'q') && (c != 'Q')) c = kbhit();
					std::cout << "\nAre you sure you want to quit rtbup? [y/n] ";
					while ((c != 'y') && (c != 'Y') && (c != 'n') && (c != 'N')) c = kbhit();
					bQuit = ((c == 'y') || (c == 'Y'));
					sleep(1);
				}
				std::cout << "\n\n";
			}
			else //bg
			{
				while (!bQuit) { if (bRestart) { sbMain.RestartMonitors(); bRestart = false; } sleep(1); }
			}
			sbMain.StopAll();
		}
		else bQuit=true;
	}

	//-------------------------------------------------------------------------
	SetStatus(false);
	WriteToLog("Stopped");
	TellUser("\nStopped rtbup\n");
    return 0;
}


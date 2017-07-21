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

#include "rthelp.h"
#include "globs.h"
#include <iostream>


void ShowCommandHelp()
{
	std::cout << "\n\nYou can restart or stop the backgrounded instance by running rtbup"
			  << "\n (again) in a console as follows:"
			  << "\n\n    $ /-path-to-/rtbup command"
			  << "\n\nCommand    Description"
			  << "\n\nhelp | ?   (the help you are reading now)"
			  << "\n\nrestart    this will cause the background process to reload monitor settings"
			  << "\n             from it's configuration file and start any new monitors, or stop"
			  << "\n             removed ones."
			  << "\n\nstop       this will cause the background process to stop all active monitors"
			  << "\n             and terminate.\n\n";
}

void ShowHelp()
{
	std::cout << "\nUsage:"
			  << "\nYou need to edit the config file to add directories and backup-paths; it contains"
			  << "\n full details on how to make the entries. Then either run rtbup"
			  << "\n from the command-line:"
			  << "\n\n    $ /-path-to-/rtbup"
			  << "\n\nor, to start it as a background process use:"
			  << "\n\n    $ nohup /-path-to-/rtbup >/dev/null 2>&1 &";
	ShowCommandHelp();
}

void ShowBackgroundHelp()
{
	std::cout << "\nAn instance of rtbup is already running in the background.\n";
	ShowCommandHelp();
}



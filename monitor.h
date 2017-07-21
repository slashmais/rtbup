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

#ifndef _rtbup_monitor_h_
#define _rtbup_monitor_h_

extern volatile bool bStopMonitors;
void* DoMonitor(void* pTP);

#endif

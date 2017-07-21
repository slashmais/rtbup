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

#ifndef _rtbup_globs_h_
#define _rtbup_globs_h_


#include <string>
#include <map>
#include <vector>
#include <sys/stat.h>


//NB: FIXED LOCATION!..
#define LOCK_FILE std::string("/var/lock/rtbup_lock.lock")


#define WHITESPACE " \t\n"

extern std::string G_CONFIG_FILE;
extern std::string G_LOG_FILE;

extern bool G_ISATTY;


void LTRIM(std::string& s, const char *sch=WHITESPACE);
void RTRIM(std::string& s, const char *sch=WHITESPACE);
void TRIM(std::string& s, const char *sch=WHITESPACE);

const std::vector<std::string> Tokenize(const std::string s, const char *delim);

bool scmp(const std::string s1, const std::string s2);
bool sicmp(const std::string s1, const std::string s2);

//String-PrintF, ..Append, Const..-in-place-
void spf(std::string &s, const std::string &fmt, ...);
void spfa(std::string &s, const std::string &fmt, ...);
const std::string cspf(const std::string &fmt, ...);

bool IsFile(std::string s);
bool IsPath(std::string s, mode_t *pmode=0);

int kbhit(void);

void TellUser(std::string sMsg);

void Startup();
void SetUpConfigLog();
void CreateConfig();
void WriteToLog(std::string sData);

const std::string LogTimeStamp();
const std::string FileTimeStamp();

pid_t GetPrevPID();
void CheckPrevInstanceLock();
bool SetStatus(bool bSet=true);

enum RTB_Status
{
	RTB_U = 0,	//running user-interactive (UI), only instance
	RTB_B,		//running backgrounded (BG), only instance
	RTB_UEU,	//running UI, existing UI
	RTB_UEB,	//running UI, existing backgrounded
	RTB_BEU,	//running BG, existing UI
	RTB_BEB,	//running BG, existing BG
};
RTB_Status GetStatus();

#define RESTART SIGUSR1
#define STOP SIGUSR2

///monitor data
struct MonitorData
{
	std::string bupdir;
	std::vector<std::string> vex;
	MonitorData() : bupdir("") { vex.clear(); }
};
typedef std::map< std::string, MonitorData > mapMonData;

void LoadConfigData(mapMonData& mm);

bool PostRestart();
bool PostQuit();


#endif

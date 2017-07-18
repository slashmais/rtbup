
#include "globs.h"

#include <unistd.h>
#include <pwd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdarg.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <iostream>


//global vars -----------------------------------------------------------------
std::string G_CONFIG_FILE;
std::string G_LOG_FILE;


bool G_ISATTY;


void LTRIM(std::string& s, const char *sch)
{
	std::string ss=sch;
	int i=0, n=s.length();
	while ((i<n) && (ss.find(s.at(i),0)!=std::string::npos)) i++;
	s = (i>0)?s.substr(i,n-i):s;
}

void RTRIM(std::string& s, const char *sch)
{
	std::string ss=sch;
	int n = s.length()-1;
	int i=n;
	while ((i>0) && (ss.find( s.at(i),0)!=std::string::npos)) i--;
	s = (i<n)?s.substr(0,i+1):s;
}
void TRIM(std::string& s, const char *sch) { LTRIM(s, sch); RTRIM(s, sch); }


const std::vector<std::string> Tokenize(const std::string s, const char *delim)
{
	std::vector<std::string> v;
	std::string tok;
	bool b=false;
	int p=0, pp=0;

	v.clear();
	if (s.length() == 0) return v;
	p = s.find(delim, pp);
	while (!b)
	{
		if (p == (int) std::string::npos)
		{
			tok = s.substr(pp);
			b=true;
		}
		else
		{
			tok = s.substr(pp, p-pp);
			pp = p+1;
			p = s.find(delim, pp);
		}
		TRIM(tok," \"");
		v.push_back(tok);
	}
	return v;
}

bool scmp(const std::string s1, const std::string s2)
{
    register char c1, c2;
    const char *l=s1.c_str(), *r=s2.c_str();
    do
    {
      c1 = *l++;
      c2 = *r++;
    } while ( c1 && (c1 == c2) );
    return ((c1 - c2)==0);
}

bool sicmp(const std::string s1, const std::string s2)
{
    register char c1, c2;
    const char *l=s1.c_str(), *r=s2.c_str();
    do
    {
      c1 = tolower(*l++);
      c2 = tolower(*r++);
    } while ( c1 && (c1 == c2) );
    return ((c1 - c2)==0);
}

void spf(std::string &s, const std::string &fmt, ...)
{
    int n, size=100;
    bool b=false;
    va_list marker;

    while (!b)
    {
        s.resize(size);
        va_start(marker, fmt);
        n = vsnprintf((char*)s.c_str(), size, fmt.c_str(), marker);
        va_end(marker);
        if ((n>0) && ((b=(n<size))==true)) s.resize(n); else size*=2;
    }
}

void spfa(std::string &s, const std::string &fmt, ...)
{
	std::string ss;
    int n, size=100;
    bool b=false;
    va_list marker;

    while (!b)
    {
        ss.resize(size);
        va_start(marker, fmt);
        n = vsnprintf((char*)ss.c_str(), size, fmt.c_str(), marker);
        va_end(marker);
        if ((n>0) && ((b=(n<size))==true)) ss.resize(n); else size*=2;
    }
	s += ss;
}
const std::string cspf(const std::string &fmt, ...)
{
	std::string ss;
    int n, size=100;
    bool b=false;
    va_list marker;

    while (!b)
    {
        ss.resize(size);
        va_start(marker, fmt);
        n = vsnprintf((char*)ss.c_str(), size, fmt.c_str(), marker);
        va_end(marker);
        if ((n>0) && ((b=(n<size))==true)) ss.resize(n); else size*=2;
    }
	return ss;
}

bool IsFile(std::string s)
{
	struct stat st;
	if (!stat(s.c_str(), &st)) return (S_ISREG(st.st_mode));
	else return false;
}

bool IsPath(std::string sDir, mode_t *pmode/*=0*/)
{
	struct stat st;
	if (!stat(sDir.c_str(), &st))
	{
		if (S_ISDIR(st.st_mode))
		{
			if (pmode) *pmode = st.st_mode;
			return true;
		}
	}
	return false;
}

void Startup()
{
	SetStatus();
	SetUpConfigLog();
}

void LoadConfigData(mapMonData& mm)
{
	std::string s, st, sS, sD, sE;
	int p=0;
	std::ifstream ifs(G_CONFIG_FILE.c_str());

	mm.clear();
	while (ifs.good())
	{
		std::getline(ifs, s);
		TRIM(s);
		if (s.length()&&(s.at(0)!='#')&&(s.at(0)!='*')) //toss comments & suspended
		{
			if ((p=s.find('=')) > 0)
			{
				sS=s.substr(0, p);
				st=s.substr(p+1);
				if ((p=st.find('|')) > 0) { sD=st.substr(0,p); sE=st.substr(p+1); }
				else { sD=st; sE=""; }
			}
			else { sS=s; sD=sE=""; }
			TRIM(sS); TRIM(sD); TRIM(sE, " []\t\n\r");
			if (IsPath(sS) && IsPath(sD) && !scmp(sS, sD))
			{
				mm[sS].bupdir = sD;
				mm[sS].vex = Tokenize(sE, ",");
			}
			else
			{
				spf(s, " - Error: (invalid source- or backup-path) entry: %s", s.c_str());
				WriteToLog(s);
				TellUser(s);
			}
		}
	}
	ifs.close();
}

void SetUpConfigLog()
{
	std::string s;
	struct passwd *pw = getpwuid(getuid());

	s = pw->pw_dir; //homedir
	s += (s.at(s.length()-1)=='/')?"":"/";
	s += ".config";
	if (!IsPath(s)) mkdir(s.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	s += "/rtbup";
	if (!IsPath(s)) mkdir(s.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	s+="/";

	G_CONFIG_FILE = s + "rtbup.config";
	G_LOG_FILE = s + "rtbup.log";

	std::ifstream ifs(G_CONFIG_FILE.c_str());
	if (!ifs.good())
	{
		ifs.close();
		CreateConfig();
	}
	else ifs.close();
}

void CreateConfig()
{
	std::ofstream ofs(G_CONFIG_FILE.c_str());
	if (ofs.good())
	{
		ofs << "# rtbup.config"
			<< "\n# -----------"
			<< "\n# Monitoring Directories:"
			<< "\n# Format of entries:"
			<< "\n# Source-path = Backup-path | [ excludes ]"
			<< "\n# A valid entry is all on one line, with Source- and Backup-paths mandatory,"
			<< "\n# and |[excludes] optional (pipe-symbol followed by excludes between square brackets)."
			<< "\n#----------------------------------------"
			<< "\n# Excludes are constructed using * and ? wildcards"
			<< "\n# Examples:"
			<< "\n# *~        exclude filenames that ends with a ~ (tilde)"
			<< "\n# *.temp.*  exclude filenames contains the phrase: .temp."
			<< "\n# init      exclude files named 'init'"
			<< "\n# ??        exclude filenames that consists of just two characters"
			<< "\n# A single * will exclude everything, and so on."
			<< "\n#--------------------------------------------------------------------------------"
			<< "\n#A monitor can be disabled/suspended by placing an asterisk * in the first column."
			<< "\n#--------------------------------------------------------------------------------"
			<< "\n#Example:"
			<< "\n# /home/myname/workdir/projectdir = /bupdisk/mybackups/projectdir|[*.~,tmp.*]"
			<< "\n# Your entries here:"
			<< "\n# -----------\n";
	}
	ofs.close();
}

void WriteToLog(std::string sData)
{
	std::string ts = LogTimeStamp();
	std::ofstream ofs(G_LOG_FILE.c_str(), std::ios_base::app);
	if (ofs.good()) ofs << "[" << ts << "] " << sData << "\n";
	ofs.close();
}

const std::string LogTimeStamp()
{
	int y,M,d,h,m,s;
	std::stringstream ss;
	time_t t;
	struct tm tmcur;

	time(&t);
	tmcur = *localtime(&t);
	y = (1900 + tmcur.tm_year);
	M = tmcur.tm_mon+1;
	d = tmcur.tm_mday;
	h = tmcur.tm_hour;
	m = tmcur.tm_min;
	s = tmcur.tm_sec;

	ss << y
		<< ((M<=9)?"0":"") << M
		<< ((d<=9)?"0":"") << d
		<< " "
		<< ((h<=9)?"0":"") << h << "h"
		<< ((m<=9)?"0":"") << m
		<< ":"
		<< ((s<=9)?"0":"") << s;

	return ss.str();
}

const std::string FileTimeStamp()
{
	long ms;
	int y,M,d,h,m,s;
	std::stringstream ss;
	time_t t;
	struct tm tmcur;
	struct timeval tv;

	time(&t);
	tmcur = *localtime(&t);
	y = (1900 + tmcur.tm_year);
	M = tmcur.tm_mon+1;
	d = tmcur.tm_mday;
	h = tmcur.tm_hour;
	m = tmcur.tm_min;
	s = tmcur.tm_sec;

	gettimeofday(&tv, NULL);
	ms = ((tv.tv_sec * 1000 + tv.tv_usec/1000.0) + 0.5);
	ms %= 1000;

	ss << y
		<< ((M<=9)?"0":"") << M
		<< ((d<=9)?"0":"") << d
		<< "-"
		<< ((h<=9)?"0":"") << h
		<< ((m<=9)?"0":"") << m
		<< ((s<=9)?"0":"") << s
		<< "-"
		<< ms;

	return ss.str();
}

int kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);

	newt = oldt;
	newt.c_lflag &= ~0000172 ; //~(ICANON | ECHO);

	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	return ch;
}

void TellUser(const std::string sMsg)
{
	if (G_ISATTY) std::cout << sMsg << "\n";
}

pid_t GetPrevPID()
{
	pid_t pid = 0;;
	std::string s = "";

	std::ifstream ifs(LOCK_FILE.c_str());
	if (ifs.good())
	{
		std::getline(ifs, s);
		std::getline(ifs, s);
		std::stringstream ss(s);
		ss >> pid;
	}
	ifs.close();
	return pid;
}

bool SetStatus(bool bSet)
{
	bool b=false;
	if (bSet)
	{
		pid_t pid = getpid();
		std::ofstream ofs(LOCK_FILE.c_str());
		if ((b = ofs.good())==true)
		{
			if (isatty(0)) ofs << "RUI\n"; else ofs << "RBG\n";
			ofs << pid << "\n";
		}
		ofs.close();
	}
	else remove(LOCK_FILE.c_str());
	return b;
}

void CheckPrevInstanceLock()
{
	pid_t ppid = GetPrevPID();
	bool b=false;
	if (ppid != 0)
	{
		std::string sf, s;
		spf(sf, "/proc/%d/cmdline", ppid);
		std::ifstream ifs(sf.c_str());
		size_t pos;
		while (!b && ifs.good())
		{
			std::getline(ifs, s);
			b = ((pos = s.rfind("rtbup")) != std::string::npos);
		}
		ifs.close();
		if (!b) remove(LOCK_FILE.c_str()); //prev instance died unexpectedly
	}
}

RTB_Status GetStatus()
{
	RTB_Status nRet;
	std::string s = "";
	bool bRUI = isatty(0);

	CheckPrevInstanceLock();

	std::ifstream ifs(LOCK_FILE.c_str());
	if (ifs.good())
	{
		std::getline(ifs, s);
		if (sicmp(s, "RUI"))
		{
			if (bRUI) nRet = RTB_UEU;
			else nRet = RTB_BEU;
		}
		else
		{
			if (bRUI) nRet = RTB_UEB;
			else nRet = RTB_BEB;
		}
	}
	else
	{
		if (bRUI) nRet = RTB_U;
		else nRet = RTB_B;
	}
	ifs.close();
	return nRet;
}

bool PostRestart()
{
	pid_t pid = GetPrevPID();
	if (pid != 0) { kill((pid_t)pid, RESTART); return true; }
	return false;
}

bool PostQuit()
{
	pid_t pid = GetPrevPID();
	if (pid != 0) { kill((pid_t)pid, STOP); return true; }
	return false;
}


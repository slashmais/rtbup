
#include "tparm.h"

#include <fstream>



THREAD_PARM::THREAD_PARM() : fdInotify(0), MB() { LoadConfigData(MB); }

bool THREAD_PARM::GotEntries() { return (MB.size()  > 0); }

bool THREAD_PARM::Excluded(const std::string sF, const std::vector<std::string> &vex)
{
	//processing wildcards..
	bool b=false;
	int v=0, fn=sF.length();

	while (!b && (v < (int)vex.size()))
	{
		std::string sw = vex.at(v++);
		int f=0, w=0, wn=sw.length();

		while ((f<fn) && (w<wn) && (sF.at(f)==sw.at(w))) { f++; w++; }
		if ((w==wn) && (f==fn)) { b = true; continue; }
		if ((w==wn) || (f>=fn)) continue;

		while ((f<fn) && (w<wn))
		{
			if (sw.at(w)=='*')
			{
				while ((w<wn) && ((sw.at(w)=='*'))) w++;
				if (w==wn) { b = true; continue; }
			}

			if (sw.at(w)=='?')
			{
				while ((w<wn) && (f<fn) && ((sw.at(w)=='?'))) { w++; f++; }
				if ((w==wn) && (f==fn)) { b = true; continue; }
				if ((w==wn) || (f==fn)) continue;
			}

			std::string sp="";
			while ((w<wn) && ((sw.at(w)!='*') && (sw.at(w)!='?')))
			{
				sp += sw.at(w);
				w++;
			}

			f = sF.find(sp, f);
			if (f == (int)std::string::npos) { f=fn+1; continue; }
			f += sp.length();
			if ((w==wn) && (f==fn)) { b = true; continue; }
			if ((w==wn) || (f==fn)) continue;
		}
		if ((w==wn) && (f==fn)) { b = true; continue; }
	}
	return b;
}

void THREAD_PARM::BackItUp(const std::string sM, const std::string sF)
{
	MonitorData *p = &(MB[sM]);
	if (Excluded(sF, p->vex)) return;

	std::string fts=FileTimeStamp(), sSDir=sM, sSFile, sBDir=p->bupdir, sFBDir, sBFile;
	mode_t mode;

	sSDir += (sSDir.at(sSDir.length()-1)=='/')?"":"/";
	spf(sSFile, "%s%s", sSDir.c_str(), sF.c_str());

	sBDir += (sBDir.at(sBDir.length()-1)=='/')?"":"/";
	if (IsPath(sBDir, &mode))
	{
		spf(sFBDir, "%sbup_%s", sBDir.c_str(), sF.c_str());
		if (!IsPath(sFBDir))
		{
			if (mkdir(sFBDir.c_str(), mode)!=0)
			{
				WriteToLog(cspf(" - Error(BackItUp): Cannot create sub-directory: '%s'", sFBDir.c_str()));
				return;
			}
			else
			{
				std::string sOrg;
				spf(sOrg, "%s/sourcefilepath", sFBDir.c_str());
				std::ofstream ofsOrg(sOrg.c_str());
				if (ofsOrg.good()) ofsOrg << sSFile << "\n";
				ofsOrg.close();
			}
		}

		spf(sBFile, "%s/%s.SB_%s", sFBDir.c_str(), sF.c_str(), fts.c_str());

		std::ifstream ifs(sSFile.c_str());
		if (ifs.good())
		{
			std::ofstream ofs(sBFile.c_str());
			if (ofs.good()) ofs << ifs.rdbuf();
			else WriteToLog(cspf(" - Error(BackItUp): Cannot write file: '%s'", sBFile.c_str()));
			ofs.close();
		}
		else WriteToLog(cspf(" - Error(BackItUp): Cannot read file: '%s'", sSFile.c_str()));
		ifs.close();
	}
	else WriteToLog(cspf(" - Error(BackItUp): Backup directory does not exist: '%s'", sBDir.c_str()));
}


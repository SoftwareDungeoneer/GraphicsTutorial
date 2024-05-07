#include "Settings.h"

#include <fstream>
#include <map>
#include <string>

using namespace std;

using ConfigSettings = map<string, string>;
using ConfigMap = map<string, ConfigSettings>;

void MapConfigToValues(const ConfigMap&, std::shared_ptr<Settings>);

// String helper functions
// trim from start (in place)
inline void ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
		}));
}

// trim from end (in place)
inline void rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
		}).base(), s.end());
}

inline void trim(std::string& s) {
	rtrim(s);
	ltrim(s);
}

Settings Settings::GetDefaultSettings()
{
	auto DefaultPosition = [](POINT& p) { p.x = p.y = CW_USEDEFAULT; };
	auto DefaultSize = [](SIZE& s) { s.cx = s.cy = CW_USEDEFAULT; };

	Settings settings;
	settings.RememberWindowPosition = 1;
	settings.RememberWindowSize = 1;

	DefaultPosition(settings.mainWindowPos);
	DefaultPosition(settings.toolWindowPos);
	DefaultPosition(settings.debugWindowPos);

	DefaultSize(settings.mainWindowSize);
	DefaultSize(settings.toolWindowSize);
	DefaultSize(settings.debugWindowSize);

	return settings;
}

// String block
constexpr char kGeneralBlock[]{ "[General]" };
constexpr char kMainWindowBlock[]{ "[Main Window]" };
constexpr char kToolWindowBlock[]{ "[Tool Window]" };
constexpr char kDebugWindowBlock[]{ "[Debug Window]" };

constexpr char kGeneralRememberWindowPositions[]{ "RememberWindowPosition" };
constexpr char kGeneralRememberWindowSize[]{ "RememberWindowSize" };

constexpr char kWindowXPos[]{ "Window XPos" };
constexpr char kWindowYPos[]{ "Window YPos" };
constexpr char kWindowCX[]{ "Window Width" };
constexpr char kWindowCY[]{ "Window Height" };

ostream& WriteWindowSettings(ostream& o, const POINT& p, const SIZE& s)
{
	o << kWindowXPos << "=" << p.x << endl;
	o << kWindowYPos << "=" << p.y << endl;
	o << kWindowCX   << "=" << s.cx << endl;
	o << kWindowCY   << "=" << s.cy << endl;
	return o;
}

int Settings::Serialize(LPCTSTR filename, const std::shared_ptr<Settings> pSettings)
{
	ofstream ofile(filename, std::ios::trunc);
	if (!ofile)
		return 0;

	// Main window options
	ofile << kGeneralBlock << endl;
	ofile << kGeneralRememberWindowPositions << "=" 
		  << pSettings->RememberWindowPosition << endl;
	ofile << kGeneralRememberWindowSize  << "=" 
		  << pSettings->RememberWindowSize << endl;

	ofile << kMainWindowBlock << endl;
	WriteWindowSettings(ofile, pSettings->mainWindowPos, pSettings->mainWindowSize);

	// Tool window settings
	ofile << kToolWindowBlock << endl;
	WriteWindowSettings(ofile, pSettings->toolWindowPos, pSettings->toolWindowSize);

	// Debug window2
	ofile << kDebugWindowBlock << endl;
	WriteWindowSettings(ofile, pSettings->debugWindowPos, pSettings->debugWindowSize);

	return 1;
}

int Settings::Deserialize(LPCTSTR filename, std::shared_ptr<Settings> pSettings)
{
	ifstream ins(filename);

	ConfigMap config;
	string currentSection = "";
	string line;
	while (getline(ins, line))
	{
		if (line[0] == '[')
		{
			currentSection = line;
			trim(currentSection);
			continue;
		}

		auto eqPos = line.find_first_of('=');
		if (string::npos == eqPos)
			break;

		string key = line.substr(0, eqPos);
		string val = line.substr(eqPos + 1);
		trim(key);
		trim(val);

		config[currentSection][key] = val;
	}

	MapConfigToValues(config, pSettings);

	return 1;
}

void MapConfigToValues(const ConfigMap& config, std::shared_ptr<Settings> settings)
{
	for (const auto& [blockStr, block] : config)
	{
		for (const auto& [settingString, value] : block)
		{
			POINT* pPoint{ nullptr };
			SIZE* pSize{ nullptr };
			if (blockStr == kGeneralBlock)
			{
				if (settingString == kGeneralRememberWindowPositions)
					settings->RememberWindowPosition = atoi(value.c_str());
				else if (settingString == kGeneralRememberWindowSize)
					settings->RememberWindowSize = atoi(value.c_str());
				continue;
			}
			
			if (blockStr == kMainWindowBlock)
			{
				pPoint = &settings->mainWindowPos;
				pSize = &settings->mainWindowSize;
			}
			else if (blockStr == kToolWindowBlock)
			{
				pPoint = &settings->toolWindowPos;
				pSize = &settings->toolWindowSize;
			}
			else if (blockStr == kDebugWindowBlock)
			{
				pPoint = &settings->debugWindowPos;
				pSize = &settings->debugWindowSize;
			}
			else
			{
				break;
			}

			if (settingString == kWindowXPos)
				pPoint->x = atoi(value.c_str());
			else if (settingString == kWindowYPos)
				pPoint->y = atoi(value.c_str());
			else if (settingString == kWindowCX)
				pSize->cx = atoi(value.c_str());
			else if (settingString == kWindowCY)
				pSize->cy = atoi(value.c_str());
		}
	}
}

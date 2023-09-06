#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "httplib.h"

#include "toml.hpp"

#include "utility.hpp"
#include "GeometryDash.hpp"

#include <unordered_map>
#include <thread>
#include <sstream>
#include <fstream>

using namespace httplib;

typedef std::unordered_map<std::string, std::string> LevelInfo;

static std::mutex g_Mtx;
static LevelInfo g_LatestLevelInfo;

static bool g_SSL = false;
static std::string g_Endpoint;
static std::string g_Body;
static std::string g_ContentType;

//Main levels

static std::string const STEREO_MADNESS =	"1:1:2:Stereo Madness:8:10:9:10:12:1:15:3:17:0:18:1:37:3:38:1:45:2288";
static std::string const BACK_ON_TRACK =	"1:2:2:Back On Track:8:10:9:10:12:2:15:3:17:0:18:2:37:3:38:1:45:1542";
static std::string const POLARGEIST =		"1:3:2:Polargeist:8:10:9:20:12:3:15:3:17:0:18:3:37:3:38:1:45:1528";
static std::string const DRY_OUT =			"1:4:2:Dry Out:8:10:9:20:12:4:15:3:17:0:18:4:37:3:38:1:45:1441";
static std::string const BASE_AFTER_BASE =	"1:5:2:Base After Base:8:10:9:30:12:5:15:3:17:0:18:5:37:3:38:1:45:2143";
static std::string const CANT_LET_GO =		"1:6:2:Cant Let Go:8:10:9:30:12:6:15:3:17:0:18:6:37:3:38:1:45:1701";
static std::string const JUMPER =			"1:7:2:Jumper:8:10:9:40:12:7:15:3:17:0:18:7:37:3:38:1:45:2810";
static std::string const TIME_MACHINE =		"1:8:2:Time Machine:8:10:9:40:12:8:15:3:17:0:18:8:37:3:38:1:45:3596";
static std::string const CYCLES =			"1:9:2:Cycles:8:10:9:40:12:9:15:3:17:0:18:9:37:3:38:1:45:3918";
static std::string const XSTEP =			"1:10:2:xStep:8:10:9:50:12:10:15:3:17:0:18:10:37:3:38:1:45:4713";
static std::string const CLUTTERFUNK =		"1:11:2:Clutterfunk:8:10:9:50:12:11:15:3:17:0:18:11:37:3:38:1:45:4860";
static std::string const TOE =				"1:12:2:Theory of Everything:8:10:9:50:12:12:15:3:17:0:18:12:37:3:38:1:45:4342";
static std::string const ELECTROMAN =		"1:13:2:Electroman Adventures:8:10:9:50:12:13:15:3:17:0:18:10:37:3:38:1:45:5640";
static std::string const CLUBSTEP =			"1:14:2:Clubstep:8:10:9:50:12:14:15:3:17:1:18:14:37:3:38:1:45:7782";
static std::string const ELECTRODYNAMIX =	"1:15:2:Electrodynamix:8:10:9:50:12:15:15:3:17:0:18:15:37:3:38:1:45:9520";
static std::string const HEXAGON =			"1:16:2:Hexagon Force:8:10:9:50:12:16:15:3:17:0:18:12:37:3:38:1:45:10221";
static std::string const BLAST =			"1:17:2:Blast Processing:8:10:9:40:12:17:15:3:17:0:18:10:37:3:38:1:45:11549";
static std::string const TOE2 =				"1:18:2:Theory of Everything 2:8:10:9:50:12:18:15:3:17:1:18:14:37:3:38:1:45:15655";
static std::string const GEOMETRICAL =		"1:19:2:Geometrical Dominator:8:10:9:40:12:19:15:3:17:0:18:10:37:3:38:1:45:16128";
static std::string const DEADLOCKED =		"1:20:2:Deadlocked:8:10:9:50:12:20:15:3:17:1:18:15:37:3:38:1:45:idk";
static std::string const FINGER =			"1:21:2:Fingerdash:8:10:9:50:12:21:15:3:17:0:18:12:37:3:38:1:45:19495";
static std::string const THE_CHALLENGE =	"1:3001:2:The Challenge";

__declspec(dllexport) void emptyStub() {} //for linking

static bool g_Enabled = true;

//Response table

static std::unordered_map<std::string, std::string> RESPONSE_TABLE =
{
	{ "1", "LEVELID" },
	{ "2", "LEVELNAME" },
	{ "3", "DESCRIPTION" },
	{ "5", "VERSION" },
	{ "6", "PLAYERID" },
	{ "8", "DIFFDEN" },
	{ "9", "DIFFNUM" },
	{ "10", "DOWNLOADS" },
	{ "12", "OFFICIALSONG" },
	{ "13", "GAMEVERSION" },
	{ "14", "LIKES" },
	{ "15", "LENGTH" },
	{ "17", "DEMON" },
	{ "18", "STARS" },
	{ "19", "FEATURESCORE" },
	{ "25", "AUTO" },
	{ "30", "COPIEDID" },
	{ "31", "TWOPLAYER" },
	{ "35", "CUSTOMSONG" },
	{ "37", "COINS" },
	{ "38", "VERIFIEDCOINS" },
	{ "39", "REQUESTEDSTARS" },
	{ "42", "EPIC" },
	{ "43", "DEMONDIFF" },
	{ "45", "OBJECTS" },
};

//Helpers

Client * getClient(std::string const& host = "", int port = 0)
{
	static Client* c = nullptr; 

	if (!c)
		c = new Client(host, port);

	return c;
}

SSLClient* getSSLClient(std::string const& host = "", int port = 0)
{
	static SSLClient* c = nullptr;

	if (!c)
	{
		c = new SSLClient(host, port);
		c->set_ca_cert_path("ca-bundle.crt");
		c->enable_server_certificate_verification(true);
	}

	return c;
}

std::vector<std::string> tokenize(std::string const& buf)
{
	std::vector<std::string> tokens;
	std::stringstream ss(buf);
	std::string s;

	while (std::getline(ss, s, ':'))
	{
		tokens.push_back(s);

		auto pos = tokens.back().find('#');

		if (pos != std::string::npos)
		{
			tokens.back().erase(
				tokens.back().begin() + pos,
				tokens.back().end());

			break;
		}
	}

	return tokens;
}

inline void getRequestString(
	std::string& s,
	uint32_t const id)
{
	s = "type=0&str=";
	s += std::to_string(id);
	s += "&secret=Wmfd2893gb7";

	return;
}

LevelInfo const& getLevelInfo()
{
	g_Mtx.lock();
	auto const& ref = g_LatestLevelInfo;
	g_Mtx.unlock();
	return ref;
}

void updateMainLevelInfo(
	Client& c,
	uint32_t const id)
{
	LevelInfo info;
	std::string lvl;

	switch (id)
	{
	case 1:
		lvl = STEREO_MADNESS;
		break;
	case 2:
		lvl = BACK_ON_TRACK;
		break;
	case 3:
		lvl = POLARGEIST;
		break;
	case 4:
		lvl = DRY_OUT;
		break;
	case 5:
		lvl = BASE_AFTER_BASE;
		break;
	case 6:
		lvl = CANT_LET_GO;
		break;
	case 7:
		lvl = JUMPER;
		break;
	case 8:
		lvl = TIME_MACHINE;
		break;
	case 9:
		lvl = CYCLES;
		break;
	case 10:
		lvl = XSTEP;
		break;
	case 11:
		lvl = CLUTTERFUNK;
		break;
	case 12:
		lvl = TOE;
		break;
	case 13:
		lvl = ELECTROMAN;
		break;
	case 14:
		lvl = CLUBSTEP;
		break;
	case 15:
		lvl = ELECTRODYNAMIX;
		break;
	case 16:
		lvl = HEXAGON;
		break;
	case 17:
		lvl = BLAST;
		break;
	case 18:
		lvl = TOE2;
		break;
	case 19:
		lvl = GEOMETRICAL;
		break;
	case 20:
		lvl = DEADLOCKED;
		break;
	case 21:
		lvl = FINGER;
		break;
	case 3001:
		lvl = THE_CHALLENGE;
		break;
	}

	auto tokens = tokenize(lvl);

	if (!(tokens.size() % 2))
	{
		for (auto i = 0u; i < tokens.size(); i += 2)
		{
			auto it = RESPONSE_TABLE.find(tokens[i]);

			if (it != RESPONSE_TABLE.end())
			{
				info.insert({ it->second, tokens[i + 1] });
			}
		}

		g_Mtx.lock();
		g_LatestLevelInfo = std::move(info);
		g_Mtx.unlock();
	}
}

void updateLevelInfo(
	Client& c,
	uint32_t const id)
{
	LevelInfo info;
	std::string request;

	if (id < 128 || id == 3001)
	{
		updateMainLevelInfo(c, id);
		return;
	}

	getRequestString(request, id);

	auto response = c.Post(
		"/database/getGJLevels21.php",
		request,
		"application/x-www-form-urlencoded");

	if (response &&
		response->status == 200 &&
		response->body != "-1")
	{
		auto tokens = tokenize(response->body);

		if (!(tokens.size() % 2))
		{
			for (auto i = 0u; i < tokens.size(); i += 2)
			{
				auto it = RESPONSE_TABLE.find(tokens[i]);

				if (it != RESPONSE_TABLE.end())
				{
					info.insert({ it->second, tokens[i + 1] });
				}
			}

			g_Mtx.lock();
			g_LatestLevelInfo = std::move(info);
			g_Mtx.unlock();
		}
	}
}

int32_t getBest()
{
	//Pointer stolen from absolute
	auto pointerBase = 0x3222D0;
	std::vector<uint32_t> pointerOffsets = { 0x164, 0x22C, 0x114, 0x248 };

	auto p = utility::getPointer(
		pointerBase,
		pointerOffsets);

	if (p)
		return *reinterpret_cast<int32_t*>(p);

	return -1;
}

int32_t getLast()
{
	//Pointer stolen from absolute
	auto pointerBase = 0x3222C0;
	std::vector<uint32_t> pointerOffsets = { 0xCC, 0x28, 0x0, 0x8, 0x4C0 };

	auto p = utility::getPointer(
		pointerBase,
		pointerOffsets);

	if (p)
		return *reinterpret_cast<int32_t*>(p);

	return -1;
}

void levelInfoUpdater()
{
	Client c("162.216.16.96", 80);
	//Pointer stolen from zmx, who stole it from absolute
	auto IDpointerBase = 0x3222D0;
	std::vector<uint32_t> IDpointerOffsets = { 0x164, 0x22C, 0x114, 0xF8 };
	//Pointers stolen from absolute
	std::vector<uint32_t> SpointerOffsets = { 0x164, 0x488, 0xFC };
	std::vector<uint32_t> SpointerOffsets2 = { 0x164, 0x488, 0xFC, 0x0 };

	while (true)
	{
		auto IDp = utility::getPointer(
			IDpointerBase,
			IDpointerOffsets);
		
		//A failed attempt at getting local level names

		/*
		auto Sp = utility::getPointer(
			IDpointerBase,
			SpointerOffsets);

		auto Sp2 = utility::getPointer(
			IDpointerBase,
			SpointerOffsets2);

		if (IDp && Sp || 
			IDp && Sp2)
		{
			LevelInfo info;
			char const* s;

			if (Sp)
			{
				s = reinterpret_cast<char const*>(Sp);
			}
			else
			{
				s = reinterpret_cast<char const*>(Sp2);
			}

			auto length = strlen(s);
			std::string cpps(s, s + length);

			if (getLevelInfo().at("LEVELID") != std::to_string(*reinterpret_cast<uint32_t*>(IDp)) ||
				getLevelInfo().at("LEVELNAME") != cpps)
			{
				info.insert({ "LEVELID", std::to_string(*reinterpret_cast<uint32_t*>(IDp)) });
				info.insert({ "LEVELNAME" , cpps });

				g_Mtx.lock();
				g_LatestLevelInfo = std::move(info);
				g_Mtx.unlock();
			}
		}
		*/
		if (IDp)
		{
			auto id = *reinterpret_cast<uint32_t*>(IDp);
			auto info = getLevelInfo();
			auto it = info.find("LEVELID");

			if (it == info.end() ||
				it->second != std::to_string(id))
			{
				updateLevelInfo(c, id);
			}
		}
	}
}

void replaceInString(std::string& s,
	std::string const& replace,
	std::string const& with)
{
	auto pos = s.find("{" + replace + "}");

	while (pos != std::string::npos)
	{
		s.replace(pos, replace.length() + 2, with);
		pos = s.find("{" + replace + "}");
	}
}

void runCommand(std::string const& cmd, std::string const& args)
{
	STARTUPINFOA si = {};
	PROCESS_INFORMATION pi = {};

	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWMINIMIZED;

	CreateProcessA(const_cast<char*>(cmd.c_str()),
		const_cast<char*>((cmd + " " + args).c_str()), NULL, NULL, FALSE, 0, 0, 0, &si, &pi);

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}

void deathHandler()
{
	if (!g_Enabled)
		return;

	auto info = getLevelInfo();
	auto body = g_Body;

	for (auto it = info.begin(); it != info.end(); ++it)
		replaceInString(body, it->first, it->second);

	replaceInString(body, "BESTPERCENT", std::to_string(getBest()));
	replaceInString(body, "LASTPERCENT", std::to_string(getLast()));

	while (!body.back())
		body.pop_back();

	if (g_ContentType == "cmd")
	{
		std::thread([body]()
			{
				runCommand(g_Endpoint, body);
			}).detach();
	}
	else
	{
		std::thread([body]() {

			if (g_SSL)
			{
				auto client = getSSLClient();

				auto r = client->Post(
					g_Endpoint.c_str(),
					body,
					g_ContentType.c_str());
			}
			else
			{
				auto client = getClient();

				client->Post(
					g_Endpoint.c_str(),
					body,
					g_ContentType.c_str());
			}
			}).detach();
	}
}

bool loadConfigFile()
{
	auto data = toml::parse("config.toml");

	bool cmd = toml::find<bool>(data, "cmd");

	if (cmd)
	{
		g_ContentType = "cmd";
		g_Endpoint = toml::find<std::string>(data, "command");
		g_Body = toml::find<std::string>(data, "args");
	}
	else
	{
		auto const host = toml::find<std::string>(data, "host");
		auto const port = toml::find<int>(data, "port");
		g_SSL = toml::find<bool>(data, "ssl");

		if (g_SSL)
		{
			getSSLClient(host, port);
		}
		else
		{
			getClient(host, port);
		}

		g_Endpoint = toml::find<std::string>(data, "endpoint");

		std::ifstream bodyData(
			toml::find<std::string>(data, "bodyPath"),
			std::ios::ate,
			std::ios::binary);

		if (!bodyData.is_open())
			return false;

		auto size = bodyData.tellg();
		bodyData.seekg(0, std::ios::beg);

		g_Body.resize(size);

		bodyData.read(g_Body.data(), size);
	}
	return true;
}

//Init function
DWORD WINAPI MainThread(LPVOID)
{
	//Load debug console
	utility::showConsole();

	//Load Config file
	if (!loadConfigFile())
	{
		utility::debugMsg("Couldn't load config file\n.");
		return 1;
	}

	//Load gd stuff
	if (!GeometryDash::init())
	{
		utility::debugMsg("Couldn't init GD stuff\n.");
		return 1;
	}

	//Setup callback
	GeometryDash::installCallback(deathHandler);
	
	//Setup levelinfo updater
	std::thread(levelInfoUpdater).detach();

	std::thread([&]()
		{
			while (true)
			{
				if (GetAsyncKeyState(VK_F5))
				{
					if (g_Enabled)
					{
						MessageBoxA(NULL, "Death detour disabled.", "Disabled", MB_OK | MB_ICONINFORMATION);
						g_Enabled = false;
					}
					else
					{
						MessageBoxA(NULL, "Death detour enabled.", "Enabled", MB_OK | MB_ICONINFORMATION);
						g_Enabled = true;
					}
				}
			}
		}).detach();

	return 0;
}

//Entrypoint of DLL
BOOL WINAPI DllMain(
	HINSTANCE const dll,
	DWORD const reason,
	LPVOID const)
{
	DisableThreadLibraryCalls(dll);

	if (reason == DLL_PROCESS_ATTACH)
		CreateThread(
			NULL,
			NULL,
			MainThread,
			NULL,
			NULL,
			NULL);

	return TRUE;
}
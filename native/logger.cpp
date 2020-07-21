
//#define STANDALONE
#include "logger.h"

using namespace klog;

#include <set>
#include <array>
#include <unordered_map>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <filesystem>
#include <cstdlib>
#include <unordered_map>
#include <sstream>
#ifdef WIN32
#include <wtsapi32.h> // vista
#include <psapi.h> // GetModuleInformation
#endif

//#include <fmt/format.h>

namespace fs = std::filesystem;

#if !defined(STANDALONE)
#include "compilerhelper.h"
#include "zipper.h"
#include "processutil.h"

#ifdef _WIN32
#include "stringhelper.h"
#include "zMiniDump.h"
#include "XMessageBox.h"
#include <shellapi.h>
#pragma comment(lib, "shell32")
#endif
using namespace util23;
#endif 

#ifdef _WIN32
#pragma comment(lib, "ntdll")
extern "C" long __stdcall RtlGetVersion(OSVERSIONINFOEXW*);
#endif


//#define USE_INDENT_OUTPUT
#ifdef  USE_INDENT_OUTPUT
static int thread_local scope_ = 0;
static const char pad[] = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

template<typename XCHAR>
static inline int xheader(XCHAR* buffer, char level, const char* tag)
{
	const int BUFSIZE = 128;
	SYSTEMTIME  ltm; GetLocalTime(&ltm);
	if constexpr (sizeof(XCHAR) == 2)
		return swprintf_s(buffer, BUFSIZE, L"%c.%-8.8S  %-8d %02d:%02d:%02d.%03d %S", level, tag, gettid(), ltm.wHour, ltm.wMinute, ltm.wSecond, ltm.wMilliseconds,
			&pad[sizeof(pad) - scope_]);
	else
		return sprintf_s(buffer, BUFSIZE, "%c.%-8.8s  %-8d %02d:%02d:%02d.%03d %s", level, tag, gettid(), ltm.wHour, ltm.wMinute, ltm.wSecond, ltm.wMilliseconds,
			&pad[sizeof(pad) - scope_]);
}


void klog::enter_scope(const char* lbl)
{
	DebugPrintf(0, 0, "{ %s", lbl);
	++scope_;
	Assert(scope_ <= 20);
}

void klog::leave_scope(const char* lbl)
{
	--scope_;
	DebugPrintf(0, 0, "} %s", lbl);
	Assert(scope_ >= 0);
}

#else // USE_INDENT_OUTPUT

template<typename XCHAR>
static inline int xheader(XCHAR* buffer, char level, const char* tag)
{
	const int BUFSIZE = 128;
	SYSTEMTIME  ltm; GetLocalTime(&ltm);
	if constexpr (sizeof(XCHAR) == 2)
		return swprintf_s(buffer, BUFSIZE, L"%c.%-8.8S  %-8d %02d:%02d:%02d.%03d   ", level, tag, gettid(), ltm.wHour, ltm.wMinute, ltm.wSecond, ltm.wMilliseconds);
	else
		return sprintf_s(buffer, BUFSIZE, "%c.%-8.8s  %-8d %02d:%02d:%02d.%03d   ", level, tag, gettid(), ltm.wHour, ltm.wMinute, ltm.wSecond, ltm.wMilliseconds);
}

void klog::enter_scope(const char* lbl)
{
	DebugPrintf(0, 0, "{ %s", lbl);
}

void klog::leave_scope(const char* lbl)
{
	DebugPrintf(0, 0, "} %s", lbl);
}

#endif // USE_INDENT_OUTPUT



klog::LogStream::LogStream(char level, const char* f, int l) : file_(f), line_(l)
{
	char buffer[128];
	xheader(buffer, level, kTag);
	oss_ << buffer;
}

klog::LogStream::~LogStream() {
	if (file_)
		oss_ << " at " << file_ << ':' << line_;
	oss_ << '\n';
	
	Out::A(oss_.str().c_str());
}


void klog::DebugPrintf(const char* file, int line, const char* format, ...)
{
	const int BUFSIZE = 256;
	const int ATSIZE = 256;
	char buffer[BUFSIZE + ATSIZE];
	int n = xheader(buffer, 'D', kTag);

	va_list va;
	va_start(va, format);
	int i = std::vsnprintf(&buffer[n], BUFSIZE - n - 1, format, va);
	if (i < (int)BUFSIZE - n - 1)
	{
		va_end(va);
#ifdef NONL
		if (buffer[n + i - 1] == '\n') --i;
#endif
		if (file)
			std::snprintf(&buffer[n + i], ATSIZE, " at %s:%d\n", file, line);
		else {
			buffer[n + i] = '\n'; // replace \0.
			buffer[n + i + 1] = 0;
		}
		Out::A(buffer);
	}
	else
	{
		std::unique_ptr<char[]> heapbuf(new char[i + n + 2 + ATSIZE]);
		memcpy(&heapbuf[0], &buffer[0], n * sizeof(char));
		std::vsnprintf(&heapbuf[n], i + n + 1, format, va);
		va_end(va);

#ifdef NONL
		if (buffer[n + i - 1] == '\n') --i;
#endif
		if (file)
			std::snprintf(&heapbuf[n + i], ATSIZE, " at %s:%d\n", file, line);
		else {
			heapbuf[n + i] = '\n'; // replace \0.
			heapbuf[n + i + 1] = 0;
		}
		Out::A(heapbuf.get());
	}
}


void klog::DebugPrintf(const char* file, int line, const wchar_t* format, ...)
{
	const int BUFSIZE = 256;
	const int ATSIZE = 256;
	wchar_t buffer[BUFSIZE + ATSIZE];
	int n = xheader(buffer, 'D', kTag);

	va_list va;
	va_start(va, format);
	int i = _vsnwprintf_s(&buffer[n], BUFSIZE, BUFSIZE - n - 2, format, va);
	if (i > 0)
	{
		va_end(va);
		if (file)
			std::swprintf(&buffer[n + i], ATSIZE, L" at %S:%d\n", file, line);
		else {
			buffer[n + i] = L'\n'; // replace \0.
			buffer[n + i + 1] = 0;
		}
		Out::W(buffer);
	}
	else
	{
		i = _vscwprintf(format, va) + n + 2;
		std::unique_ptr<wchar_t[]> heapbuf(new wchar_t[i + ATSIZE]);
		memcpy(&heapbuf[0], &buffer[0], n * sizeof(wchar_t));

		i = _vsnwprintf_s(&heapbuf[n], i - n - 2, i - n - 2, format, va);

		va_end(va);
		if (file)
			std::swprintf(&heapbuf[n + i], ATSIZE, L" at %S:%d\n", file, line);
		else {
			heapbuf[n + i] = L'\n';
			heapbuf[n + i + 1] = 0;
		}

		Out::W(heapbuf.get());
	}
}


enum { max_path = 260 };

void klog::AssertStream::createDumpfile(const char* file, int line)
{
#if defined(_WIN32) && !defined(STANDALONE)
	::util23::mswin::CreateUserDump(file, line);
#endif
}

static std::string getExecutablePath()
{
	std::array<char, max_path> buf;
#ifdef _WIN32
	Assert(GetModuleFileNameA(nullptr, buf.data(), (DWORD)buf.size()) > 0);
#else
	Assert(readlink("/proc/self/exe", buf.data(), buf.size()) > 0);
#endif
	return std::string(buf.data());
}


#pragma warning(disable:4722)
klog::AssertStream::~AssertStream()
{
#ifdef STANDALONE
	log_.reset(); // flush
#elif defined(_WIN32)
	log_->stream() << '\n' << util23::GetCallStack(10, 1);
	log_->stream() << '\n' << util23::GetMemoryState();
	auto msg = log_->str();
	log_.reset(); // flush

	XMSGBOXPARAMS xmb = {};
	xmb.lpReportUserData = &msg;
	xmb.lpReportFunc = [](LPCTSTR msg, void* userData) {
		klog::ZipLogfiles(msg);
	};
	strcpy_s(xmb.szReportButtonCaption, sizeof(xmb.szReportButtonCaption), "Send log files");

	::XMessageBox(nullptr, msg.c_str(), "ASSERTION FAIL", MB_ICONERROR, &xmb);
#else
	log_->stream() << '\n' << util23::GetCallStack(10, 1);
	log_->stream() << '\n' << util23::GetMemoryState();
	log_.reset(); // flush
#endif
	//std::abort();
	std::exit(-1);
}
#pragma warning(default:4722)



#include <fstream>
#include <filesystem>


void klog::BackupLog(const std::string& logPath, uint32_t max_size)
{
	namespace fs = std::filesystem;
	//try 
	{
		if (fs::file_size(logPath) > max_size)
		{
			std::error_code ec;
			fs::rename(fs::path(logPath).replace_extension("backup2.log"), fs::path(logPath).replace_extension("backup3.log"), ec); // 2->3
			fs::rename(fs::path(logPath).replace_extension("backup1.log"), fs::path(logPath).replace_extension("backup2.log"), ec); // 1->2
			fs::rename(logPath, fs::path(logPath).replace_extension("backup1.log"), ec); // cur.log ->1.
		}
	}
	// catch (fs::filesystem_error & e) {
	// 	//std::cout << e.what() << '\n';
	// }
}

// g2tray.exe-20190628.142825-S2.objects-P7076.SYSTEM.log
std::string klog::GetLogPath(const std::string& subfolder, std::string logName)
{
	fs::path exePath(getExecutablePath());
#ifdef WIN32
	fs::path logDir = std::getenv("USERPROFILE");
	logDir /= "Documents";
#else
	fs::path logDir = exePath.parent_path();
#endif

	if (!subfolder.empty())
	{
		std::error_code ec;
		
		logDir /= subfolder;
		if (fs::create_directories(logDir, ec) || ec.value() == 0)
		{
			// clear old logs.
			const int MAX_LOG_FILES = 200;
			std::unordered_map<uint64_t, std::string> files;
			fs::directory_iterator endit;
			for (fs::directory_iterator iter(logDir); iter != endit; ++iter)
			{
				if (!fs::is_regular_file(*iter)) continue;
				if (iter->path().extension() != ".log") continue;
				auto tick = iter->last_write_time().time_since_epoch().count();
				files[tick] = iter->path().string();
			}

			int haveToDelete = (int)files.size() - MAX_LOG_FILES;
			for (auto& [tick, path] : files) {
				if (haveToDelete-- <= 0) break;
				LOGI0 << "file removed: " << path;
				std::remove(path.c_str());
			}
		}
		else
			LOGE << ec.message() << logDir.string();
	}

	if (logName.empty())
		logName = exePath.filename().string();

	{
		std::string userNamePID = [] {
#ifdef _WIN32
			if (HANDLE hToken = 0; OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
			{
				std::array<char, 300>tubuf;
				auto ptu = (TOKEN_USER*)tubuf.data();
				DWORD dummy;
				if (GetTokenInformation(hToken, TokenUser, ptu, (DWORD)tubuf.size(), &dummy))
				{
					CloseHandle(hToken);
					int iUse;
					std::array<char, 300> domain, name;
					DWORD dlen = (DWORD)domain.size();
					DWORD nlen = (DWORD)name.size();
					if (LookupAccountSidA(0, ptu->User.Sid, name.data(), &nlen, domain.data(), &dlen, (PSID_NAME_USE)&iUse))
					{
						//return domain.data() + std::string(".") + name.data();
						return std::string(name.data());
					}
				}
			}
#endif
			return std::string{};
		}();

		std::string userNameSID = [] {
#ifdef _WIN32
			std::string name;
			{
				DWORD sid = 0;
				::ProcessIdToSessionId(getPid(), &sid);

				DWORD bytesReturned = 0;
				if (char* pData = NULL; WTSQuerySessionInformationA(WTS_CURRENT_SERVER_HANDLE, sid, WTSUserName, &pData, &bytesReturned)) {
					name = pData;
					WTSFreeMemory(pData);
				}
			}
			return name;
#endif
			return std::string{};
		}();

		userNameSID = userNamePID = ""; // disable username.
		std::array<char, 128> logext;
		DWORD sid = 0;
		auto pid = getPid();
		ProcessIdToSessionId(pid, &sid);
		time_t t = time(0); struct tm now; localtime_s(&now, &t);
		snprintf(logext.data(), logext.size(), "-%04d%02d%02d.%02d%02d%02d-S%d.%s-P%d.%s.log",
			now.tm_year + 1900, now.tm_mon + 1, now.tm_mday,
			now.tm_hour, now.tm_min, now.tm_sec,
			sid, userNameSID.c_str(),
			pid, userNamePID.c_str());

		logName += logext.data();
	}

	return (logDir / logName).string();
}


FILE* klog::OpenFile(const std::string& path)
{
	FILE* fp;
#ifdef _WIN32
	fp = _fsopen(path.c_str(), "wt", _SH_DENYWR);
#else
	fopen_s(&fp, path.c_str(), "wt");
#endif
	Assert(fp) << fmt::csprintf("Can not open file %s", path.c_str());
	Assert(0 == setvbuf(fp, nullptr, _IONBF, 0)); // set nobuffer.
	return fp;
}



std::string klog::GetHeader(void* handle, const char* buildtime)
{
	std::ostringstream ossbuf;
	ossbuf << "\n---------------------------------------------------------";

#if defined(STANDALONE) && defined(_WIN32)
	// osversion.
	{
		ossbuf << "\n\t";
		OSVERSIONINFOEXW osver = { sizeof(osver) };
		RtlGetVersion(&osver);

#		define OSENT(major,minor,server, str)	{(major<<16)|(minor)|(server), str}
		std::unordered_map<uint32_t, const char*> osname = {
			 OSENT(10, 0, 0, "Win10"),
			 OSENT(10, 0, 1 << 31, "Server2016"),
			 OSENT(6,  3, 0, "Win8.1"),
			 OSENT(6 , 3, 1 << 31, "Server2012 R2"),
			 OSENT(6,  2, 0, "Win8"),
			 OSENT(6,  2, 1 << 31, "Server2012"),
			 OSENT(6,  1, 0, "Win7"),
			 OSENT(6,  1, 1 << 31, "Server2008 R2"),
			 OSENT(6,  0, 1 << 31, "Server2008"),
			 OSENT(6,  0, 0, "Vista"),
			 OSENT(5,  2, 1 << 31, "Server2003 R2"),// GetSystemMetrics(SM_SERVERR2) != 0
			 OSENT(5,  2, 0, "Server2003"), // GetSystemMetrics(SM_SERVERR2) == 0
			 OSENT(5,  1, 0, "WinXP"),
		};

		bool isServer = (osver.wProductType != VER_NT_WORKSTATION);
		if (osver.dwMajorVersion == 5 && osver.dwMinorVersion == 2)
			isServer = (GetSystemMetrics(SM_SERVERR2) != 0);
		
		if (uint32_t id = (osver.dwMajorVersion << 16) | osver.dwMinorVersion | (isServer ? (1 << 31) : 0); osname.find(id) != osname.end())
			ossbuf << osname[id];

		SYSTEM_INFO si = {};
		::GetNativeSystemInfo(&si);
		ossbuf << ((si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) ? "-x64" : "-x86");

		ossbuf << fmt::csprintf("-%d.%d sp.%d build.%d ", osver.dwMajorVersion, osver.dwMinorVersion, osver.wServicePackMajor, osver.dwBuildNumber);

		if (HKEY key; 0 == RegOpenKeyExA(HKEY_LOCAL_MACHINE, R"(SOFTWARE\Microsoft\Windows NT\CurrentVersion)", 0, KEY_READ, &key))
		{
			char buf[512];
			if (DWORD size = sizeof(buf); 0 == ::RegQueryValueExA(key, "ReleaseId", 0, NULL, (LPBYTE)buf, &size)) {
				ossbuf << buf;
			}
			if (DWORD size = sizeof(buf); 0 == ::RegQueryValueExA(key, "BuildLabEx", 0, NULL, (LPBYTE)buf, &size)) {
				ossbuf <<"/" << buf;
			}
			else if (DWORD size = sizeof(buf); 0 == ::RegQueryValueExA(key, "BuildLab", 0, NULL, (LPBYTE)buf, &size)) {
				ossbuf <<"/" << buf;
			}
			RegCloseKey(key);
		}
	}

	// computer/user name/session/desktop
	{
		ossbuf << "\n\t";
		//char buf[200] = "";
		// if (DWORD  charSize = sizeof(buf); GetComputerNameA(buf, &charSize))
		// 	ossbuf << buf;
		// if (DWORD  charSize = sizeof(buf); GetUserNameA(buf, &charSize))
		// 	ossbuf << "/" << buf;
		ossbuf << " SID=" << WTSGetActiveConsoleSessionId();

		HDESK hInputDesk = ::OpenInputDesktop(0, FALSE, GENERIC_ALL);
		if (hInputDesk) {
			DWORD dummy;
			char name[256] = {};
			::GetUserObjectInformationA(hInputDesk, UOI_NAME, name, sizeof(name), &dummy);
			::CloseDesktop(hInputDesk);
			ossbuf << ", InputDesktop=" << name;
		}
	}
	// uptime
	{
		uint64_t tick = GetTickCount64() / 1000;
		uint64_t days = tick / (60 * 60 * 24);
		tick -= days * (60 * 60 * 24);
		uint64_t hour = tick / (60 * 60);
		tick -= hour * (60 * 60);
		uint64_t minute = (tick + 30) / (60);
		ossbuf << fmt::csprintf("\n\tUptime: %d days, %d hours, %d minutes", (uint32_t)days, (uint32_t)hour, (uint32_t)minute);
	}

	// CPU
	{
		SYSTEM_INFO si = {};
		GetSystemInfo(&si);
		ossbuf << fmt::csprintf("\n\tProcessor: Core=%d, Type=%d, OemID=%d, Mask=0x%X", si.dwNumberOfProcessors, si.dwProcessorType, si.dwOemId, (uint32_t)si.dwActiveProcessorMask);
	}

	{ // module range.
		auto moduleinfo = [](HMODULE handle) {
			MODULEINFO  mi = {};
			char buf[400], path[MAX_PATH] = {};
			::GetModuleInformation(GetCurrentProcess(), handle, &mi, sizeof(mi));
			GetModuleFileNameA(handle, path, sizeof(path));
			sprintf_s(buf, sizeof(buf), "Range=%p~%p, %s", mi.lpBaseOfDll, (char*)mi.lpBaseOfDll + mi.SizeOfImage, path);
			return std::string(buf);
		};

		ossbuf << "\n\t" << moduleinfo(::GetModuleHandleA(nullptr));
		if (handle)
			ossbuf << "\n\t" << moduleinfo((HMODULE)handle);
	}
#else //  defined(STANDALONE) && defined(_WIN32)

	// os info.
	// module path, exe path,
	// execution time.
	ossbuf << "\nSystem information:" << util23::GetSystemInfo();
	ossbuf << "\nProcess information:" << util23::GetProcessInfo();
	ossbuf << "\n\t" << util23::GetModuleInfo(nullptr);

	if (handle)
		ossbuf << "\n\t" << util23::GetModuleInfo((HMODULE)handle);
#endif
	ossbuf << "\n\t" << buildtime;
	ossbuf << "\n---------------------------------------------------------\n";
	return ossbuf.str();
}



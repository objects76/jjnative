

#include "zMiniDump.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <unordered_map>
#include <vector>
#include <tuple>
#include <time.h>
#include <stdio.h>
#include <sstream>
#include <memory>
#include <array>

#include <filesystem>
namespace fs = std::filesystem;

#include <Strsafe.h>
#include <tlhelp32.h>
#include <Excpt.h>
#include <eh.h>

#pragma warning(disable: 4091)
#include <DbgHelp.h>
#pragma warning(default: 4091)
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "Version.lib")

//#pragma optimize("y", off)		// generate stack frame pointers for all functions - same as /Oy- in the project
#pragma warning(disable: 4200)	// nonstandard extension used : zero-sized array in struct/union
#pragma warning(disable: 4100)	// unreferenced formal parameter

#include "logger.h"
using namespace klog;




using namespace util23;
using namespace util23::mswin;
namespace util23
{
	std::string GetModulePathFromAddrAWithoutError(const void* ptr); // for callstack dup.
	std::string getModuleVersion(const std::string& szModulePath);

	static std::string getCurrentTime() {
		time_t t = time(0); struct tm now; localtime_s(&now, &t);
		return fmt::csprintf("%04d%02d%02d.%02d%02d%02d",
			now.tm_year + 1900, now.tm_mon + 1, now.tm_mday,
			now.tm_hour, now.tm_min, now.tm_sec);
	}


	// path, baseaddr
	std::pair<std::string, void*> getModuleInfoFromAddrA(const void* symbolAddress);

	namespace mswin
	{
		struct SymInit
		{
			SymInit::SymInit()
			{
				if (!SymInitialize(GetCurrentProcess(), nullptr, TRUE))
					LOGERRNO() << "sym init";
				DWORD symOptions = SymGetOptions(); // +DbgHelp.dll 5.1
				symOptions |= SYMOPT_LOAD_LINES;
				symOptions &= ~SYMOPT_UNDNAME;
				SymSetOptions(symOptions);
			}

			~SymInit() {
				SymCleanup(GetCurrentProcess());
			}
		};
		std::string getSymbol(const void* opAddress);
		bool		mineIsToplevel();

		void		createMiniDump(EXCEPTION_POINTERS* pep, const char* dmpName, uint32_t threadId = gettid());

		std::string dumpFolder;


		
		static struct {
			_invalid_parameter_handler	_crt_param_handler;
			_invalid_parameter_handler	_thread_local_crt_param_handler;
			_purecall_handler			_crt_purecall_handler;
			LPTOP_LEVEL_EXCEPTION_FILTER _seh_handler;

			HMODULE		_hModule;
			bool		is_handler_installed;
			void*		old_seh_address;
			DWORD		old_seh_code;
			uint32_t	old_seh_tid;
		} cxt_ = {};
	}
}

#define DEV		(0)
static bool sehDumpCallStack(LPEXCEPTION_POINTERS exptr, std::ostream& oss, int depth = 10)
{
	FNSCOPE_IF(DEV);
	int skip = 0;
	if (!exptr || exptr->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
		return false;

	// util23::win::SymInit si; called from caller.
	auto process = GetCurrentProcess();

	CONTEXT cxt = *(exptr->ContextRecord);
	STACKFRAME64 sf = {};
	sf.AddrPC.Mode = AddrModeFlat;
	sf.AddrStack.Mode = AddrModeFlat;
	sf.AddrFrame.Mode = AddrModeFlat;
#ifdef _M_IX86  // Intel Only!
	sf.AddrPC.Offset = cxt.Eip;
	sf.AddrStack.Offset = cxt.Esp;
	sf.AddrFrame.Offset = cxt.Ebp;
	DWORD MachineType = IMAGE_FILE_MACHINE_I386;
#elif defined(_M_X64)  // Intel Only!
	sf.AddrPC.Offset = cxt.Rip;
	sf.AddrStack.Offset = cxt.Rsp;
	sf.AddrFrame.Offset = cxt.Rbp;
	DWORD MachineType = IMAGE_FILE_MACHINE_AMD64;
#endif

	DWORD64 symoffset = 0;
	DWORD   lineoffset = 0;

	std::vector<char> tmpbuf(sizeof(SYMBOL_INFO) + MAX_SYM_NAME, 0);
	SYMBOL_INFO* symbol = (SYMBOL_INFO *)tmpbuf.data();
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	symbol->MaxNameLen = MAX_SYM_NAME;

	//DWORD64 pPrevFrame = 0;
	for (int num = 0; num < depth; ++num)
	{
		SetLastError(0);
		if (!StackWalk64(MachineType,
			process, GetCurrentThread(),
			&sf,
			&cxt,
			NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
			break;

		// Sanity check
		if (sf.AddrPC.Offset == 0) break;
		//if ((sf.AddrPC.Offset % sizeof(void *)) != 0) break;
		//if (pPrevFrame)
		//{
		//	if (sf.AddrPC.Offset <= pPrevFrame) break;
		//	if ((sf.AddrPC.Offset - pPrevFrame) > 10000000) break;
		//}
		//pPrevFrame = sf.AddrPC.Offset;

		if (num >= skip) // && pcOffset != sf.AddrPC.Offset)
		{
			//std::string modulePathR = util23::GetModulePathFromAddrA((void*)sf.AddrPC.Offset);
			//LOGE2(0,0) <<  "    => %p: at %s:%d", (void*)sf.AddrPC.Offset, modulePathR, 0);

			char func_name[512] = {};
			if (SymFromAddr(process, sf.AddrPC.Offset, &symoffset, symbol))
				::UnDecorateSymbolName(symbol->Name, func_name, 511, UNDNAME_COMPLETE);

			// at.
			std::string modulePath;
			IMAGEHLP_LINE64 line = { sizeof(IMAGEHLP_LINE64) };
			if (SymGetLineFromAddr64(process, sf.AddrPC.Offset, (DWORD*)&lineoffset, &line) == FALSE)
			{
				modulePath = util23::GetModulePathFromAddrAWithoutError((void*)sf.AddrPC.Offset);
				line.FileName = (char*)modulePath.c_str();
			}

			strcpy_s(&func_name[80], 5, "...");
			oss << fmt::csprintf("\t\t=> %p:%30s at %s:%d\n", (void*)sf.AddrPC.Offset, func_name, line.FileName, line.LineNumber);
			//LOGI << "pc=0x%llx, symoffset=0x%llx, lineoffset=%d", stackframe.AddrPC.Offset, symoffset, lineoffset);
		}
	}
	return true;
}


static void sehDumpExceptionInfo(PEXCEPTION_POINTERS pException, std::ostream& oss)
{
	std::vector<char> szBuf(1024 * 4, 0);
	char exePath[MAX_PATH] = { 0, };
	GetModuleFileNameA(nullptr, exePath, MAX_PATH);


	oss <<
		fmt::csprintf("Process: %s(PID=%d)", exePath, getPid());

	// If exception occurred.
	if (pException)
	{
		EXCEPTION_RECORD &	exRec = *pException->ExceptionRecord;



		std::string modulePath;
		void* pModAddr;
		std::tie(modulePath, pModAddr) = util23::getModuleInfoFromAddrA((PBYTE)exRec.ExceptionAddress);

		if (pModAddr)
		{
			oss << "\n"
				<< "Module: " << modulePath;
		}

		oss << "\n"
			<< fmt::csprintf("Exception Addr: %p, Exception Code: 0x%X", exRec.ExceptionAddress, exRec.ExceptionCode);

		if (exRec.ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
		{
			oss << "\n"
				<< fmt::csprintf("%s Address:  %p",
				(exRec.ExceptionInformation[0] == 0) ? "Read" :
					(exRec.ExceptionInformation[0] == 1) ? "Write" : "DEP",
					(void*)exRec.ExceptionInformation[1]);

		}

		// Save instruction that caused exception.
		oss << "\n" << "Instruction: ";

		PBYTE pCode = (PBYTE)(exRec.ExceptionAddress);
		for (int i = 0; i < 16; i++) {
			if (IsBadReadPtr(pCode + i, 1)) break;
			oss << fmt::csprintf(" %02X", pCode[i]);
		}

		// Save registers at exception.
		const CONTEXT& C = *pException->ContextRecord;
		oss << "\n" << "Registers:";
#ifdef _WIN64
		oss << "\n" << fmt::csprintf("RAX: %016X  EBX: %016X  ECX: %016X  EDX: %016X", C.Rax, C.Rbx, C.Rcx, C.Rdx);
		oss << "\n" << fmt::csprintf("RSI: %016X  EDI: %016X  ESP: %016X  EBP: %016X", C.Rsi, C.Rdi, C.Rsp, C.Rbp);
		oss << "\n" << fmt::csprintf("RIP: %016X  EFlags: 0x%X", C.Rip, C.EFlags);
#else
		oss << "\n" << fmt::csprintf("EAX: %08X  EBX: %08X  ECX: %08X  EDX: %08X", C.Eax, C.Ebx, C.Ecx, C.Edx);
		oss << "\n" << fmt::csprintf("ESI: %08X  EDI: %08X  ESP: %08X  EBP: %08X", C.Esi, C.Edi, C.Esp, C.Ebp);
		oss << "\n" << fmt::csprintf("EIP: %08X  EFlags: 0x%X", C.Eip, C.EFlags);
#endif
	} //if (pException)
	oss << "\n-------------------------";
}



static BOOL CALLBACK dumpCallback(
	PVOID                            pParam,
	const PMINIDUMP_CALLBACK_INPUT   pInput,
	PMINIDUMP_CALLBACK_OUTPUT        pOutput)
{
	if (pInput == nullptr || pOutput == nullptr) return FALSE;
	switch (pInput->CallbackType)
	{
	case ModuleCallback:
		if (pParam != nullptr)
		{
			if (pOutput->ModuleWriteFlags & ModuleWriteDataSeg && pInput->Module.FullPath) {
				wchar_t* dmpPath = wcsrchr(pInput->Module.FullPath, L'\\'); 
				dmpPath = dmpPath ? dmpPath + 1 : pInput->Module.FullPath;
				if (_wcsicmp(dmpPath, (wchar_t*)pParam) != 0)
					pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg); // Not include global variables(data section).
			}
		}
		else
		{
			// for minidump
			if (!(pOutput->ModuleWriteFlags & ModuleReferencedByMemory))
			{
				//LOGD << L"Excluding module: %s", pInput->Module.FullPath);
				pOutput->ModuleWriteFlags &= (~ModuleWriteModule);
			}
		}
	case IncludeModuleCallback:  // Include the module into the dump 
	case IncludeThreadCallback: // Include the thread into the dump 
	case ThreadCallback: // Include all thread information into the minidump 
	case ThreadExCallback: // Include this information 
		return TRUE;
	default:
		break;
	}
	return FALSE;
};


void mswin::SetDumpFolder(const char* subfolder, void* hModule)
{
	Assert(subfolder && *subfolder);

	fs::path baseDir = std::getenv("USERPROFILE");
	baseDir /= "Documents";
	baseDir /= subfolder;

	std::error_code ec;
	if (fs::create_directories(baseDir, ec) || ec.value() == 0)
	{
		mswin::dumpFolder = baseDir.string();

		// clear old logs.
		const int MAX_DMP_FILES = 50;
		std::unordered_map<uint64_t, std::string> files;
		fs::directory_iterator endit;
		for (fs::directory_iterator iter(baseDir); iter != endit; ++iter)
		{
			if (!fs::is_regular_file(*iter)) continue;
			if (iter->path().extension() != ".dmp") continue;
			auto tick = iter->last_write_time().time_since_epoch().count();
			files[tick] = iter->path().string();
		}

		int haveToDelete = (int)files.size() - MAX_DMP_FILES;
		for (auto&[tick, path] : files) {
			if (haveToDelete-- <= 0) break;
			LOGI0 << "file removed: " << path;
			std::remove(path.c_str());
		}
	}
	else
		LOGE << ec.message() << baseDir.string();
}


static std::string getDumpPath(const char* dumpFilename)
{
	
	if (mswin::dumpFolder.empty())
	{
		std::array<char, 512> path;
		GetModuleFileNameA(nullptr, path.data(), path.size());

		fs::path baseDir = fs::path(path.data()).parent_path();
		mswin::dumpFolder = baseDir.string();
	}

	return mswin::dumpFolder + '\\' + dumpFilename;
}


static inline auto dyfn_get(const std::string_view& lib, const std::string_view& fn) {
#ifdef _WIN32
	HMODULE dll = GetModuleHandleA(lib.data());
	if (!dll) dll = LoadLibraryA(lib.data());
	return dll ? GetProcAddress(dll, fn.data()) : nullptr;
#else
	void* dll = dlopen(lib.c_str(), RTLD_NOW);
	return dll ? (farproc)dlsym(dll, fn.c_str()) : nullptr;
#endif
}

void mswin::createMiniDump(EXCEPTION_POINTERS* pep, const char* dumpFilename, uint32_t threadId)
{
	Assert(pep);
	Assert(dumpFilename);

	std::string dumpPath = getDumpPath(dumpFilename);
	auto miniDumpWriteDump = (decltype(&MiniDumpWriteDump))dyfn_get("dbghelp", "MiniDumpWriteDump");
	if (!miniDumpWriteDump) {
		LOGW << " No MiniDumpWriteDump";
		return;
	}

	HANDLE hFile = CreateFileA(dumpPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)0);

	if (hFile == INVALID_HANDLE_VALUE)
		return;

	MINIDUMP_TYPE dumpType = (MINIDUMP_TYPE)(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory);
	MINIDUMP_CALLBACK_INFORMATION mci = {};
	mci.CallbackRoutine = dumpCallback;

	MINIDUMP_EXCEPTION_INFORMATION exInfo = {};
	exInfo.ThreadId = threadId;
	exInfo.ExceptionPointers = pep;
	exInfo.ClientPointers = FALSE;
	std::unique_ptr<MINIDUMP_USER_STREAM_INFORMATION> musi;
	BOOL rv = miniDumpWriteDump(GetCurrentProcess(), getPid(), hFile,
		dumpType,
		(pep != 0) ? &exInfo : 0,
		musi.get(), &mci);
	if (!rv) LOGERRNO() << "Create dump";

	CloseHandle(hFile);
}

void util23::mswin::CreateFullDump(const char* dumpFilename)
{
	std::string dumpPath = getDumpPath(dumpFilename);

	auto miniDumpWriteDump = (decltype(&MiniDumpWriteDump))dyfn_get("dbghelp", "MiniDumpWriteDump");
	if (!miniDumpWriteDump) return;


	HANDLE hFile = CreateFileA(dumpPath.c_str(), GENERIC_ALL, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		return;

	MINIDUMP_TYPE dumpType = (MINIDUMP_TYPE)(MiniDumpWithFullMemory |
		MiniDumpWithFullMemoryInfo |
		MiniDumpWithHandleData |
		MiniDumpWithThreadInfo |
		MiniDumpWithUnloadedModules);
	BOOL Result = miniDumpWriteDump(GetCurrentProcess(), getPid(), hFile, dumpType, nullptr, nullptr, nullptr);
	if (!Result) LOGERRNO() << "Create dump";
	CloseHandle(hFile);
}

//
//
//
template<typename T1, typename T2>
static const T1* strstrT(const T1* hatch, const T2* needle, bool ignoreCase = false)
{
	if (*needle == 0)
		return nullptr;

	auto isEqual = [](int c1, int c2, bool ignoreCase) {
		int diff = c1 - c2;
		return diff == 0 || (ignoreCase && (diff == 0x20 || diff == -0x20));
	};

	while (*hatch != 0)
	{
		if (isEqual(*hatch, *needle, ignoreCase))
		{
			auto h_pos = hatch + 1;
			auto n_pos = needle + 1;
			while (*n_pos != 0 && *h_pos != 0 && isEqual(*h_pos, *n_pos, ignoreCase)) {
				h_pos++;
				n_pos++;
			}

			if (*n_pos == 0) return hatch;
			if (*h_pos == 0) break;
		}
		hatch++;
	}
	return nullptr;
}
static BOOL CALLBACK midiCallback(char* module_list, const MINIDUMP_CALLBACK_INPUT* pInput, MINIDUMP_CALLBACK_OUTPUT* pOutput)
{
	if (pInput == 0 || pOutput == 0)
		return FALSE;
	switch (pInput->CallbackType)
	{
	case ModuleCallback:
		if (pOutput->ModuleWriteFlags & ModuleWriteDataSeg) // is valid data section?
		{
			if (auto tmp = wcsrchr(pInput->Module.FullPath, L'\\'))
			{
				if (!strstrT(module_list, tmp + 1, true)) // if not roi.
					pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg);
			}
		}
	case IncludeModuleCallback: // Include the module into the dump 
	case IncludeThreadCallback: // Include the thread into the dump 
	case ThreadCallback: // Include all thread information into the minidump 
	case ThreadExCallback: // Include this information 
		return TRUE;

	case MemoryCallback: // We do not include any information here.
	case CancelCallback:
	default:
		break;
	}

	return FALSE;
}



//  Global variables, 
// the contents of heaps and TLS, PEB, TEBs. 
// handle information and inspect the virtual memory layout.
void util23::mswin::CreateMidiDump(EXCEPTION_POINTERS* pep, const char* dumpFilename, const char* roi_modules, uint32_t threadId)
{
	Assert(pep);
	Assert(dumpFilename);


	std::string dumpPath = getDumpPath(dumpFilename);

	auto miniDumpWriteDump = (decltype(&MiniDumpWriteDump))dyfn_get("dbghelp", "MiniDumpWriteDump");
	if (!miniDumpWriteDump) return;


	HANDLE hFile = CreateFileA(dumpPath.c_str(), GENERIC_ALL, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		return;

	// Create the minidump 
	MINIDUMP_EXCEPTION_INFORMATION mdei = {};
	mdei.ThreadId = (threadId == 0 ? gettid() : threadId);
	mdei.ExceptionPointers = pep;
	mdei.ClientPointers = FALSE;

	MINIDUMP_CALLBACK_INFORMATION mci = {};
	mci.CallbackRoutine = (MINIDUMP_CALLBACK_ROUTINE)midiCallback;
	mci.CallbackParam = (PVOID)(roi_modules ? roi_modules : "");

	MINIDUMP_TYPE dumpType = (MINIDUMP_TYPE)(MiniDumpWithPrivateReadWriteMemory |
		MiniDumpWithDataSegs |
		MiniDumpWithHandleData |
		MiniDumpWithFullMemoryInfo |
		MiniDumpWithThreadInfo |
		MiniDumpWithUnloadedModules);

	BOOL rv = miniDumpWriteDump(GetCurrentProcess(), getPid(), hFile, dumpType, (pep != 0) ? &mdei : 0, 0, &mci);

	if (!rv) LOGERRNO() << "Create dump";
	CloseHandle(hFile);
}


// seh handler
//
void util23::mswin::Handle(EXCEPTION_POINTERS* exinfo, const char* dmpLabel, bool bCreateDump)
{
	FNSCOPE_IF(DEV);

	Assert(exinfo);
	std::string msg;
	{
		LogStream log('E');
		auto& oss = log.stream();
		auto excode = exinfo->ExceptionRecord->ExceptionCode;
		auto eip = exinfo->ExceptionRecord->ExceptionAddress;
		if (excode < 0x80000000) {
			oss << fmt::csprintf("--- SEH: code=%08X, eip=%p", excode, eip);
			return;
		}

		oss << "\n" << fmt::csprintf("--- SEH: %08X(pid=%d), eip=%p", excode, getPid(), eip);

		util23::mswin::SymInit si;
		oss << "\n" << fmt::csprintf("--- %s, %s", getSymbol(eip).c_str(), util23::GetModulePathFromAddrA(eip).c_str());
		oss << "\n";
		sehDumpExceptionInfo(exinfo, oss);
		oss << "\n";
		sehDumpCallStack(exinfo, oss);

		if (bCreateDump)
		{
			std::string dmpName;
			{
				std::array<char, MAX_PATH> buf;
				if (0 == GetModuleFileNameA(NULL, buf.data(), MAX_PATH))
					GetModuleFileNameA(nullptr, buf.data(), MAX_PATH);
				auto p = strrchr(buf.data(), '\\'); p = p ? p + 1 : buf.data();
				dmpName = fmt::csprintf("%s-%s-%s-pid%d.dmp", p, (dmpLabel ? dmpLabel : ""), getCurrentTime().c_str(), getPid());
			}

			oss << "\n" << fmt::csprintf("dump: %s", dmpName.c_str());

			if (excode == EXCEPTION_STACK_OVERFLOW) {
				typedef struct {
					EXCEPTION_POINTERS* pep;
					const char* dumpName;
					const char* ROIModule; // = nullptr means minidump, otherwise MIDI dump.
					const uint32_t threadId;
				} DUMP_CONTEXT;
				DUMP_CONTEXT cxt = { exinfo, dmpName.c_str(), nullptr, (uint32_t)gettid() };
				HANDLE hThread = ::CreateThread(NULL, 0, [](LPVOID lp)->DWORD {
					DUMP_CONTEXT* cxt = (DUMP_CONTEXT*)lp;
					mswin::createMiniDump(cxt->pep, cxt->dumpName, cxt->threadId);
					return 0;
				}, &cxt, 0, NULL);
				Assert(hThread);
				::WaitForSingleObject(hThread, INFINITE);
				::CloseHandle(hThread);
			}
			else {
				mswin::createMiniDump(exinfo, dmpName.c_str(), gettid());
			}
		}

		msg = log.str();
	}

	//klog::ZipLogfiles(msg.c_str());
}


// NOTE: function scope static lamda crashed in xp-sp3.
static LONG __stdcall unhandled_seh_handler(PEXCEPTION_POINTERS exinfo)
{
	FNSCOPE_IF(DEV);
	DWORD excode = exinfo->ExceptionRecord->ExceptionCode;
	PVOID eip = exinfo->ExceptionRecord->ExceptionAddress;
	if (excode >= 0x80000000)
	{
		// Prevent SE propagation.
		if (cxt_.old_seh_code != 0)
		{
			LOGE << fmt::csprintf("Another exception: excode=0x%X, eip=0x%X", excode, eip);
			LOGE << fmt::csprintf("Old SException on TID.%d: addr=%p, excode=0x%X", cxt_.old_seh_tid, cxt_.old_seh_address, cxt_.old_seh_code);
			LOGE << "Aborted here to prevent exception propagation.";
			abort();
		}
		cxt_.old_seh_address = eip;
		cxt_.old_seh_code = excode;
		cxt_.old_seh_tid = gettid();

		mswin::Handle(exinfo, nullptr, true);
	}
	if (!cxt_._seh_handler)
		return EXCEPTION_CONTINUE_SEARCH;

	// call preceding handler and log result.
	LONG result = cxt_._seh_handler(exinfo);

	if (-1 <= result && result <= 1)
	{
		const char* result_name[] = {
		"CONTINUE_EXECUTION",
		"CONTINUE_SEARCH",
		"EXECUTE_HANDLER",
		};
		LOGD0 << fmt::csprintf("handler.%p return %s", (void*)cxt_._seh_handler, result_name[result + 1]);
	}
	else
	{
		LOGE0 << fmt::csprintf("handler.%p return %d", (void*)cxt_._seh_handler, result);
	}

	return result;
};

static void __cdecl crtPurecallHandler()
{
	FNSCOPE_IF(DEV);
	_CONTEXT ContextRecord = {};
	RtlCaptureContext(&ContextRecord);

	EXCEPTION_RECORD exrec = {};
	exrec.ExceptionCode = STATUS_INVALID_CRUNTIME_PARAMETER;
#ifdef _WIN64
	exrec.ExceptionAddress = (PVOID)ContextRecord.Rip;
#else
	exrec.ExceptionAddress = (PVOID)ContextRecord.Eip;
#endif

	EXCEPTION_POINTERS exptr = {};
	exptr.ExceptionRecord = &exrec;
	exptr.ContextRecord = &ContextRecord;
	unhandled_seh_handler(&exptr);
}

static void __cdecl crtHandler(
	const wchar_t* expression,
	const wchar_t* function,
	const wchar_t* file, unsigned int line, uintptr_t pReserved)
{
	FNSCOPE_IF(DEV);
	auto wstr = fmt::csprintf(L"Invalid crt args %s:%s at %s:%d", function, expression, file, line);
	LOGE0 << ToUtf8(wstr);
	crtPurecallHandler();
}

bool mswin::RestoreUnhandledSEHandler()
{
	FNSCOPE();
	if (!(cxt_.is_handler_installed))
		return false;

	// restore original SEH.
	::SetUnhandledExceptionFilter(cxt_._seh_handler);
	_set_invalid_parameter_handler(cxt_._crt_param_handler);
	_set_thread_local_invalid_parameter_handler(cxt_._thread_local_crt_param_handler);
	_set_purecall_handler(cxt_._crt_purecall_handler);

	memset(&cxt_, 0, sizeof(cxt_));
	return true;
}
bool mswin::SetUnhandledSEHandler()
{
	if (cxt_.is_handler_installed)
		return false;

	Assert(cxt_._seh_handler == nullptr);
	Assert(cxt_._crt_param_handler == nullptr);


	// install seh.
	cxt_._seh_handler = ::SetUnhandledExceptionFilter(unhandled_seh_handler);
	//_CrtSetReportMode(_CRT_ASSERT, 0); // Disable the message box for assertions.
	if (!(DEV)) {
		cxt_._crt_param_handler = _set_invalid_parameter_handler(crtHandler);
		cxt_._thread_local_crt_param_handler = _set_thread_local_invalid_parameter_handler(crtHandler);
		cxt_._crt_purecall_handler = _set_purecall_handler(crtPurecallHandler);
	}
	LOGI0 << fmt::csprintf("old exhandlers(seh/crt): %p(%s)/%p,%p,%p",
		(void*)cxt_._seh_handler, util23::GetModulePathFromAddrA(cxt_._seh_handler).c_str(),
		(void*)cxt_._crt_param_handler,
		(void*)cxt_._thread_local_crt_param_handler,
		(void*)cxt_._crt_purecall_handler);

	cxt_.is_handler_installed = true;
	return true;
}


bool mswin::mineIsToplevel()
{
	auto toplevelFilter = ::SetUnhandledExceptionFilter([](PEXCEPTION_POINTERS exp)->LONG {return EXCEPTION_CONTINUE_SEARCH; });
	bool result = (toplevelFilter == unhandled_seh_handler);
	SetUnhandledExceptionFilter(toplevelFilter);

	if (!result) {
		LOGW << fmt::csprintf("Current top: %p, mine: %p", (void*)toplevelFilter, (void*)unhandled_seh_handler);
	}
	return result;
}


void mswin::CreateUserDump(const char* file, int line)
{
	__try {
		RaiseException(*((uint32_t*)"udmp"), 0, 0, 0); // user dump
	}
	__except ([file, line](EXCEPTION_POINTERS* exptr) {
		auto p = strrchr(file, '\\'); p = p ? p + 1 : file;
		auto t = std::time(nullptr);
		auto dmpName = fmt::csprintf("%s.%d-%s-pid%d.tid%d.dmp",
			p, line, getCurrentTime().c_str(), getPid(), gettid());
		mswin::createMiniDump(exptr, dmpName.c_str(), gettid());
		return EXCEPTION_EXECUTE_HANDLER;
	}(exception_info())) {}
}


std::string util23::mswin::getSymbol(const void* ptr)
{
	FNSCOPE_IF(DEV);
	DWORD64 dwDisplacement = 0;

	if (DEV) LOGI << "symbol ptr:" << ptr;

	std::vector<char> tmpbuf(sizeof(SYMBOL_INFO) + MAX_SYM_NAME, 0);
	SYMBOL_INFO* symbol = (SYMBOL_INFO *)tmpbuf.data();
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	symbol->MaxNameLen = MAX_SYM_NAME;

	if (DEV) LOGI << "tracing...";
	std::vector<char> undecoratedName(1024);

	if (DEV) LOGI << "tracing...";
	if (SymFromAddr(GetCurrentProcess(), (DWORD64)ptr, &dwDisplacement, symbol))
	{
		if (DEV) LOGI << "tracing...";
		int n = ::UnDecorateSymbolName(symbol->Name, undecoratedName.data(), (DWORD)undecoratedName.size(), UNDNAME_COMPLETE);
		if (DEV) LOGI << "tracing: " << n << ", " << undecoratedName.data();
		if (n > 0) strcpy_s(symbol->Name, tmpbuf.size() - sizeof(SYMBOL_INFO), undecoratedName.data());
	}
	else
	{
		if (DEV) LOGI << "tracing...";
		strcpy_s(symbol->Name, tmpbuf.size() - sizeof(SYMBOL_INFO), "???");
	}

	if (DEV) LOGI << "tracing...";
	IMAGEHLP_LINE64 line = { sizeof(IMAGEHLP_LINE64) };
	if (SymGetLineFromAddr64(GetCurrentProcess(), (DWORD64)ptr, (DWORD*)&dwDisplacement, &line) == TRUE)
	{
		std::snprintf(symbol->Name + strlen(symbol->Name), 256, " at %s:%d", line.FileName, line.LineNumber);
	}

	if (DEV) LOGI << "tracing...";
	return symbol->Name;
}

#endif // WIN32




std::string util23::getModuleVersion(const std::string& szModulePath)
{
#ifdef _WIN32
	DWORD  verHandle = 0;
	DWORD  verSize = GetFileVersionInfoSizeA(szModulePath.c_str(), &verHandle);
	if (verSize == 0) return "";

	std::vector<char> verData(verSize, 0);
	VS_FIXEDFILEINFO* verInfo = NULL;
	if (GetFileVersionInfoA(szModulePath.c_str(), verHandle, verSize, verData.data()))
	{
		UINT size = 0;
		if (VerQueryValueA(verData.data(), "\\", (void**)&verInfo, &size) && size > 0)
		{
			if (verInfo->dwSignature == 0xFEEF04BD)
			{
				// Doesn't matter if you are on 32 bit or 64 bit,
				// DWORD is always 32 bits, so first two revision numbers
				// come from dwFileVersionMS, last two come from dwFileVersionLS
				return fmt::csprintf("%d.%d.%d.%d",
					(verInfo->dwProductVersionMS >> 16) & 0xffff,
					(verInfo->dwProductVersionMS >> 0) & 0xffff,
					(verInfo->dwProductVersionLS >> 16) & 0xffff,
					(verInfo->dwProductVersionLS >> 0) & 0xffff);
			}
		}
	}
#else defined(__APPLE__)
#endif // __APPLE__
	return "";
}

// <path, base>
std::pair<std::string, void*> util23::getModuleInfoFromAddrA(const void* symbolAddress)
{
#ifdef _WIN32
	HMODULE hModule = nullptr;
	DWORD flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
	if (!GetModuleHandleExA(flags, (char*)symbolAddress, &hModule)) {
		LOGERRNO() << fmt::csprintf("GetModuleHandleExA(%p)", symbolAddress);
		return std::make_pair("", nullptr);
	}

	std::array<char, MAX_PATH> path;
	int n = GetModuleFileNameA(hModule, path.data(), path.size());
	if (n == 0) LOGERRNO() << fmt::csprintf("GetModuleFileNameA(%p)", (void*)hModule);
	return std::make_pair(path.data(), hModule);
#else
	Dl_info info;
	if (!dladdr(symbolAddress, &info)) {
		LOGE << "dladdr:" << dlerror();
			return false;
	}
	return std::string(info.dli_fname);
#endif
}

std::string util23::GetModulePathFromAddrA(const void* symbolAddress) {

	std::string path;
	void* base;
	std::tie(path, base) = getModuleInfoFromAddrA(symbolAddress);
	auto ver = getModuleVersion(path);
	return fmt::csprintf("%s(base:%p,ver=%s)", path.c_str(), base, ver.c_str());
}


std::string util23::GetModulePathFromAddrAWithoutError(const void* symbolAddress) {

	std::string path;
	void* base;
	auto getModuleInfoFromAddrA = [](const void* addr)->std::pair<std::string, HMODULE>
	{
#ifdef _WIN32
		HMODULE hModule = nullptr;
		DWORD flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
		if (!GetModuleHandleExA(flags, (char*)addr, &hModule))
		{
			return std::make_pair(std::error_code(GetLastError(), std::system_category()).message(), nullptr);
		}

		char path[MAX_PATH];
		int n = GetModuleFileNameA(hModule, path, MAX_PATH);
		if (n == 0)
			sprintf_s(path, MAX_PATH, "GetPath Failed: %p, errno=%d", hModule, GetLastError());

		return std::make_pair(path, hModule);
#else
		Dl_info info;
		if (!dladdr(symbolAddress, &info)) {
			LOGE << "dladdr: " << dlerror();
				return false;
		}
		return std::string(info.dli_fname);
#endif
	};
	std::tie(path, base) = getModuleInfoFromAddrA(symbolAddress);

	auto ver = base ? getModuleVersion(path) : "";

	return fmt::csprintf("%s(base:%p,ver=%s)", path.c_str(), base, ver.c_str());
}



std::string util23::GetCallStack(int depth, int skip)
{
	std::ostringstream osbuf;
	//std::ostringstream oss;
#ifdef _WIN32
	std::vector<void*> callstacks(depth);
	int n = ::RtlCaptureStackBackTrace(skip + 1, depth, callstacks.data(), NULL); // 1 means caller of GetCallStack.


	SymInit si;
	std::vector<char> tmpbuf(sizeof(::SYMBOL_INFO) + MAX_SYM_NAME, 0);
	for (int i = 0; i < n; ++i)
	{
		void* address = callstacks[i];
		char symbolName[256] = {};
		std::string modulePath;

		::SYMBOL_INFO* symbol = (::SYMBOL_INFO *)tmpbuf.data();
		symbol->SizeOfStruct = sizeof(::SYMBOL_INFO);
		symbol->MaxNameLen = MAX_SYM_NAME;

		DWORD64 symoffset = 0;
		if (::SymFromAddr(GetCurrentProcess(), (DWORD64)address, &symoffset, symbol))
			::UnDecorateSymbolName(symbol->Name, symbolName, 256-1, UNDNAME_COMPLETE);

		// at.
		::IMAGEHLP_LINE64 line = { sizeof(::IMAGEHLP_LINE64) };
		if (::SymGetLineFromAddr64(GetCurrentProcess(), (DWORD64)address, (DWORD*)&symoffset, &line) == FALSE)
		{
			modulePath = util23::GetModulePathFromAddrAWithoutError(address);
			line.FileName = (char*)modulePath.c_str();
		}

		if (symbolName[80]) 
			strcpy_s(&symbolName[80], 5, "..."); // truncate long name.
		osbuf << fmt::csprintf("\t\t=> %p:%30s at %s:%d\n", address, symbolName, line.FileName, line.LineNumber);
	}
#else
	depth += skip;
	void* callstack[15] = {};
	int frames = backtrace(callstack, 15);
	if (frames > depth) frames = depth;
	char** strs = backtrace_symbols(callstack, frames);
	for (int i = skip; i < frames; ++i) {
		os << fmt::csprintf("[%d] %p - %s", i, callstack[i], strs[i]);
	}
	free(strs);
#endif
	return osbuf.str();
}


void util23::DumpCallstack(const char* label, int depth, int skip)
{
	LogStream log('D');
	if (label) log.stream() << "Callstack at " << label << '\n';
	log.stream() << GetCallStack(depth, skip + 1);
}

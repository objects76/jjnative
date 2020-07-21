#pragma once

// windows only: compile with /GS(Buffer Security Check) option.
#ifdef _WIN32
#pragma strict_gs_check(push, on)
#pragma strict_gs_check(pop)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include <string>

namespace util23
{
	void DumpCallstack(const char* label, int maxdeep = 10, int skip = 0);
	std::string GetModulePathFromAddrA(const void* ptr);
	std::string GetCallStack(int maxdeep = 10, int skip = 0);

	namespace mswin
	{
		void		SetDumpFolder(const char* folder, void* hModule = nullptr);

		void		CreateUserDump(const char* file, int line);
		void		Handle(EXCEPTION_POINTERS* pep, const char* dmpLabel, bool dumpFile = false);
		// roi_modules: all modules are lower chars, start with a separator.. ex): ntdll.dll,aaa.exe\bb.dll
		void		CreateMidiDump(EXCEPTION_POINTERS* pep, const char* dumpFilename, const char* roi_modules = nullptr, uint32_t threadId = 0);
		void		CreateFullDump(const char* dumpFilename);

		// hModule(null): restore original handler
		bool		SetUnhandledSEHandler();
		bool		RestoreUnhandledSEHandler();

		//
		// seh safe function call
		//----------------------------------------------------------
		//	try {
		//		util23::sehsafe_call(seh_test, 0);
		//	}
		//	catch (const std::exception& e) {
		//		LOGE << "std::runtime_error:" << e.what();
		//	}
		//
		template<typename R, typename... Args>
		inline R sehsafe_call(R(*func)(Args...), Args... args) {
			__try {
				return func(std::forward<Args>(args)...);
			}
			__except ([](EXCEPTION_POINTERS* exp)->LONG {
				DWORD ecode = exp->ExceptionRecord->ExceptionCode;
				if (ecode == 0xE06D7363) return EXCEPTION_CONTINUE_SEARCH; // 0xE0MSC
				util23::mswin::Handle(exp, "sehsafe", true);

				throw std::runtime_error(fmt::format("seh(excode={:#x}), {}", ecode, typeid(func).name()));
				return EXCEPTION_EXECUTE_HANDLER;
			}(exception_info())) {
			}
			return R();
		}


		template<typename T, typename R, typename... Args>
		inline R sehsafe_call(T* inst, R(T::*func)(Args...), Args... args) {
			__try {
				return (inst->*func)(std::forward<Args>(args)...);
			}
			__except ([](EXCEPTION_POINTERS* exp)->LONG {
				DWORD ecode = exp->ExceptionRecord->ExceptionCode;
				if (ecode == 0xE06D7363) return EXCEPTION_CONTINUE_SEARCH; // 0xE0MSC

				util23::mswin::Handle(exp, "sehsafe", true);
				throw std::runtime_error(fmt::format("seh(excode={:#x}), {}", ecode, typeid(func).name()));
				return EXCEPTION_EXECUTE_HANDLER;
			}(exception_info())) {
			}
			return R();
		}
	}


	
} // util23

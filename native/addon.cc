

#include <napi.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "zMiniDump.h"
#endif

#include "napi-helper.hpp"
//
// { util codes
//
#include "log.h"

#define expect_type(obj, type, info)                    \
    if ((obj).Type() != type)                           \
    {                                                   \
        js_throw_error(__FUNCTION__, "Expected " #type);   \
        return info.Env().Undefined();                  \
    }

#define ThrowErrIfFailed(c, info)    if (!(c)) {  \
        js_throw_error(__FUNCTION__, #c);      \
        return info.Env().Undefined(); \
    } 

static thread_local napi_env _env = nullptr;
void js_fatal_error(std::string_view pos, std::string_view msg)
{
    std::ostringstream oss;
    LOGE0 << msg << " at " << pos;
    Napi::Error::Fatal(pos.data(), msg.data());
}
void js_throw_error(std::string_view pos, std::string_view msg)
{
    std::ostringstream oss;
    oss << "exception: " << msg << " at " << pos;
    LOGE0 << oss.str();
    if (_env) {
        Napi::Error::New(_env, oss.str().c_str()).ThrowAsJavaScriptException();
    }
}
//
// } util codes
//

bool startKeybdMonitor(int64_t hwndNumber);
bool stopKeybdMonitor();
bool pauseResumeKeybdMonitor(bool pause);

static Napi::Value startKeyMonitor(const Napi::CallbackInfo &info)
{
    
    ThrowErrIfFailed(info.Length() == 1, info);

    expect_type(info[0], napi_valuetype::napi_bigint, info);

    bool lossless = false;
    auto jsNumber = AsArg<Napi::BigInt>(info[0]);
    devlog("number= 0x%x, lossless=%d", jsNumber.Uint64Value(&lossless), lossless);

    auto result = startKeybdMonitor(jsNumber.Uint64Value(&lossless));

    return Napi::Value::From(info.Env(), result);
}

static Napi::Value stopKeyMonitor(const Napi::CallbackInfo &info)
{
    _env = info.Env();
    ThrowErrIfFailed(info.Length() == 0, info);

    auto result = stopKeybdMonitor();
    return Napi::Value::From(info.Env(), result);
}
static Napi::Value pauseKeyMonitor(const Napi::CallbackInfo &info)
{
    _env = info.Env();
    ThrowErrIfFailed(info.Length() == 0, info);

    auto result = pauseResumeKeybdMonitor(true);
    return Napi::Value::From(info.Env(), result);
}
static Napi::Value resumeKeyMonitor(const Napi::CallbackInfo &info)
{
    _env = info.Env();
    ThrowErrIfFailed(info.Length() == 0, info);
    
    auto result = pauseResumeKeybdMonitor(false);
    return Napi::Value::From(info.Env(), result);
}

static Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    _env = env;

    klog::Init();


    static FILE* fp = klog::OpenFile(klog::GetLogPath("logs", TOSTR(NODE_GYP_MODULE_NAME)) );
	klog::Out::A = [](const char* msg) {
		::OutputDebugStringA(msg);
		if (fp) fputs(msg, fp);
	};
	klog::Out::W = [](const wchar_t* msg) {
		::OutputDebugStringW(msg);
		if (fp) ::fputws(msg, fp);
	};

    void* hDllModule = nullptr;
#ifdef WIN32
	DWORD flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
	EXPECT( GetModuleHandleExA(flags, (LPSTR)&js_fatal_error, (HMODULE*)&hDllModule) );
#endif

	LOGI << klog::GetHeader(hDllModule);
    #ifdef WIN32
    util23::mswin::SetUnhandledSEHandler();
    util23::mswin::SetDumpFolder("logs");
    #endif
	FNSCOPE();

    const char *nodeenv = std::getenv("NODE_ENV");
    isDev = nodeenv && strcmp(nodeenv, "development") == 0;
    if (isDev) LOGI << "*** development mode ***";

    exports["startKeyMonitor"] = Napi::Function::New(env, startKeyMonitor);
    exports["stopKeyMonitor"] = Napi::Function::New(env, stopKeyMonitor);
    exports["pauseKeyMonitor"] = Napi::Function::New(env, pauseKeyMonitor);
    exports["resumeKeyMonitor"] = Napi::Function::New(env, resumeKeyMonitor);

    // test
    Napi::Value getPrimeAsync(const Napi::CallbackInfo &info); // promise.cc
    Napi::Value getPrimeSync(const Napi::CallbackInfo &info);
    exports["getPrimeAsync"] = Napi::Function::New(env, getPrimeAsync);
    exports["getPrimeSync"] = Napi::Function::New(env, getPrimeSync);

    // export class from native.
    Napi::Object Register_NativeAddon(Napi::Env env, Napi::Object exports); // native-class.cc
    exports = Register_NativeAddon(env, exports);

    // arguments.cc
    Napi::Value ArrayBufferArgument(const Napi::CallbackInfo &info);
    exports["ArrayBufferArgument"] = Napi::Function::New(env, ArrayBufferArgument);

    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init);

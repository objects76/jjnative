

#include <napi.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <cstdio>
#include <cstdlib>

#include "napi-helper.hpp"
//
// { util codes
//
#include "jslog.h"

#define expect_type(obj, type, info)                     \
    if ((obj).Type() != type)                            \
    {                                                    \
        js_throw_error(__FUNCTION__, "Expected " #type); \
        return info.Env().Undefined();                   \
    }

#define ThrowErrIfFailed(c, info)         \
    if (!(c))                             \
    {                                     \
        js_throw_error(__FUNCTION__, #c); \
        return info.Env().Undefined();    \
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
    if (_env)
    {
        Napi::Error::New(_env, oss.str().c_str()).ThrowAsJavaScriptException();
    }
}
static Napi::FunctionReference _jslog;
void jslog(const char *pos, int line, const char *fmt, ...)
{
    char buffer[4096];
    int n = std::snprintf(buffer, sizeof(buffer), "[N] ");
    va_list args;
    va_start(args, fmt);
    n += std::vsnprintf(buffer + n, sizeof(buffer) - n - 1, fmt, args);
    va_end(args);

    if (pos)
    {
        std::snprintf(buffer + n, sizeof(buffer) - n - 1, " at %s:%d", pos, line);
    }

    if (_jslog.IsEmpty())
        LOGI0 << buffer;
    else
    {
        _jslog.Call({Napi::String::New(_jslog.Env(), buffer)});
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
    JSLOG("number= 0x%x, lossless=%d", jsNumber.Uint64Value(&lossless), lossless);

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

static Napi::Value initLib(const Napi::CallbackInfo &info)
{
    _env = info.Env();
    ThrowErrIfFailed(info.Length() == 2, info);
    // set logger to add log to js logger system.

    _jslog.Reset();
    if (auto &jv = info[0]; !jv.IsEmpty() && !jv.IsNull())
        _jslog = AsArg<decltype(_jslog)>(jv);

    std::string logDir;
    if (auto &jv = info[1]; !jv.IsEmpty() && !jv.IsNull())
        logDir = AsArg<std::string>(jv);

    static FILE *fp = nullptr;
    if (fp)
        fclose(fp);

    auto t = localtime2();
    auto logName = fmt::csprintf(TOSTR(NODE_GYP_MODULE_NAME) ",%04d-%02d-%02d,%02-%02d-%02d,%d.log",
                                 t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec,
                                 getpid());
    auto logPath = logDir + (char)std::filesystem::path::preferred_separator + logName;
    fp = klog::fopen(logPath);
    klog::Out::A = [](const char *msg) {
#ifdef _WIN32
        ::OutputDebugStringA(msg);
#else
        std::fputs(msg, stdout);
#endif
        if (fp)
            std::fputs(msg, fp);
    };

    LOGI0 << "";
    LOGI0 << "";
    LOGI0 << "";
    LOGI0 << "----------------------------------------------";
    FNSCOPE();

    const char *nodeenv = std::getenv("NODE_ENV");
    isDev = nodeenv && strcmp(nodeenv, "development") == 0;
    if (isDev)
        LOGI << "*** development mode ***";

    return Napi::Value::From(info.Env(), true);
}

static Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports["init"] = Napi::Function::New(env, initLib);

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

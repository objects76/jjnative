

#include <napi.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

//
// { util codes
//
#include "log.h"

#define expect_type(obj, type)                                            \
    if ((obj).Type() != type)                                             \
    {                                                                     \
        throw_error("Expected " #type, __FUNCTION__ ":" TOSTR(__LINE__)); \
        return Napi::Env(_env).Undefined();                               \
    }
    
static thread_local napi_env _env = nullptr;

void fatal_error(const std::string &msg, const std::string &pos)
{
    Napi::Error::Fatal(pos.c_str(), msg.c_str());
}

void throw_error(const std::string &msg, const std::string &pos)
{
    if (_env)
    {
        auto errmsg = msg + " at " + pos;
        log("exception: %s", errmsg.c_str());
        Napi::Error::New(_env, errmsg.c_str()).ThrowAsJavaScriptException();
    }
}
//
// } util codes
//

bool startKeybdMonitor(int64_t hwndNumber);
bool stopKeybdMonitor();
bool pauseResumeKeybdMonitor(bool resume);

static Napi::Value startKeyMonitor(const Napi::CallbackInfo &info)
{
    _env = info.Env();
    if (info.Length() != 1)
    {
        Napi::Error::New(info.Env(), "Buffer object is needed.").ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }

    expect_type(info[0], napi_valuetype::napi_bigint);

    bool lossless = false;
    auto jsNumber = info[0].As<Napi::BigInt>();
    devlog("number= 0x%x, lossless=%d\n", jsNumber.Uint64Value(&lossless), lossless);

    auto result = startKeybdMonitor(jsNumber.Uint64Value(&lossless));

    return Napi::Boolean::New(info.Env(), result);
}

static Napi::Value stopKeyMonitor(const Napi::CallbackInfo &info)
{
    _env = info.Env();
    auto result = stopKeybdMonitor();
    return Napi::Boolean::New(info.Env(), result);
}
static Napi::Value pauseKeyMonitor(const Napi::CallbackInfo &info)
{
    _env = info.Env();
    auto result = pauseResumeKeybdMonitor(false);
    return Napi::Boolean::New(info.Env(), result);
}
static Napi::Value resumeKeyMonitor(const Napi::CallbackInfo &info)
{
    _env = info.Env();
    auto result = pauseResumeKeybdMonitor(true);
    return Napi::Boolean::New(info.Env(), result);
}

static Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    if (const char *env = std::getenv("NODE_ENV"))
    {
        isDev = strcmp(env, "development") == 0;
        devlog("*** development mode ***\n");
    }

    exports["startKeyMonitor"] = Napi::Function::New(env, startKeyMonitor);
    exports["stopKeyMonitor"] = Napi::Function::New(env, stopKeyMonitor);
    exports["pauseKeyMonitor"] = Napi::Function::New(env, pauseKeyMonitor);
    exports["resumeKeyMonitor"] = Napi::Function::New(env, resumeKeyMonitor);
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init);

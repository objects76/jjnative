
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <napi.h>

bool startKeybdMonitor(int64_t hwndNumber);
bool stopKeybdMonitor();
bool pauseResumeKeybdMonitor(bool resume);

static Napi::Value startKeyMonitor(const Napi::CallbackInfo &info)
{
    if (info.Length() != 1)
    {
        Napi::Error::New(info.Env(), "Buffer object is needed.").ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }

    if (!info[0].IsNumber())
    {
        Napi::Error::New(info.Env(), "Expected an Buffer").ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }

    auto jsNumber = info[0].As<Napi::Number>();
    auto result = startKeybdMonitor(jsNumber.Int64Value());
    return Napi::Boolean::New(info.Env(), result);
}

static Napi::Value stopKeyMonitor(const Napi::CallbackInfo &info)
{
    auto result = stopKeybdMonitor();
    return Napi::Boolean::New(info.Env(), result);
}
static Napi::Value pauseKeyMonitor(const Napi::CallbackInfo &info)
{
    auto result = pauseResumeKeybdMonitor(false);
    return Napi::Boolean::New(info.Env(), result);
}
static Napi::Value resumeKeyMonitor(const Napi::CallbackInfo &info)
{
    auto result = pauseResumeKeybdMonitor(true);
    return Napi::Boolean::New(info.Env(), result);
}

static Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports["startKeyMonitor"] = Napi::Function::New(env, startKeyMonitor);
    exports["stopKeyMonitor"] = Napi::Function::New(env, stopKeyMonitor);
    exports["pauseKeyMonitor"] = Napi::Function::New(env, pauseKeyMonitor);
    exports["resumeKeyMonitor"] = Napi::Function::New(env, resumeKeyMonitor);
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init);
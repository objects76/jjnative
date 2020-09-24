
#include <napi.h>
#include "jslog.h"
#include "napi-helper.hpp"

class NativeAddon : public Napi::ObjectWrap<NativeAddon>
{
public:
    static Napi::Object Register(Napi::Env env, Napi::Object exports)
    {
        FNSCOPE();
        Napi::HandleScope scope(env);

        Napi::Function ctor = DefineClass(env, "NativeAddon", {
                                                                  InstanceMethod("tryCallByStoredReference", &NativeAddon::TryCallByStoredReference),
                                                                  InstanceMethod("tryCallByStoredFunction", &NativeAddon::TryCallByStoredFunction),
                                                                  StaticMethod("staticMethod1", &NativeAddon::Dump),
                                                              });

        static Napi::FunctionReference constructor;
        constructor = Napi::Persistent(ctor);
        constructor.SuppressDestruct();

        exports["NativeAddon"] = ctor;
        //exports["dumpNativeAddon"] = Napi::Function::New(env, &NativeAddon::Dump);
        return exports;
    }
    NativeAddon(const Napi::CallbackInfo &info);

    static void Dump(const Napi::CallbackInfo &info)
    {
        FNSCOPE();
        auto _this = AsObj<NativeAddon>(info[0]);
        //std::string arg = info[1].As<Napi::String>().Utf8Value();
        auto arg = AsArg<std::string>(info[1]);
        LOGI << fmt::csprintf("dump: jsfnref=%p, jsfn=%p: %s", &_this->jsFnRef, &_this->jsFn, arg.c_str());
    }

    Napi::Value TryCallByStoredReference(const Napi::CallbackInfo &info);
    Napi::Value TryCallByStoredFunction(const Napi::CallbackInfo &info);

private:
    Napi::FunctionReference jsFnRef;
    Napi::Function jsFn;
};

Napi::Object Register_NativeAddon(Napi::Env env, Napi::Object exports)
{
    FNSCOPE();
    return NativeAddon::Register(env, exports);
}

NativeAddon::NativeAddon(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<NativeAddon>(info)
{
    FNSCOPE();
    jsFnRef = AsArg<decltype(jsFnRef)>(info[0]);
    jsFn = AsArg<decltype(jsFn)>(info[1]);

    LOGI << fmt::csprintf("ctor of NativeAddon: jsfnref=%p, jsfn=%p", &jsFnRef, &jsFn);

    if (AsArg<bool>(info[2]))
    {
        LOGI << "crash dump file generation test";
        int *p = (int *)4;
        *p = 100;
    }
}

Napi::Value NativeAddon::TryCallByStoredReference(const Napi::CallbackInfo &info)
{
    // Napi::Env env = info.Env();
    jsFnRef.Call({});

    return Napi::String::New(info.Env(), "from TryCallByStoredReference");
}

Napi::Value NativeAddon::TryCallByStoredFunction(const Napi::CallbackInfo &info)
{
    // Napi::Env env = info.Env();
    jsFn.Call({});

    return Napi::String::New(info.Env(), "from TryCallByStoredFunction");
}

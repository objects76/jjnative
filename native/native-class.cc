
#include <napi.h>
#include "log.h"
#include "napi-helper.hpp"

class NativeAddon : public Napi::ObjectWrap<NativeAddon>
{
public:
    static Napi::Object Register(Napi::Env env, Napi::Object exports) {
         Napi::HandleScope scope(env);

        Napi::Function ctor = DefineClass(env, "NativeAddon", {
            InstanceMethod("tryCallByStoredReference", &NativeAddon::TryCallByStoredReference), 
            InstanceMethod("tryCallByStoredFunction", &NativeAddon::TryCallByStoredFunction),
            //StaticMethod("NativeAddon_dump1", &NativeAddon::Dump),
            //StaticMethod("NativeAddon_dump2", &NativeAddon::Dump),
        });


        static Napi::FunctionReference constructor;
        constructor = Napi::Persistent(ctor);
        constructor.SuppressDestruct();

        exports["NativeAddon"] = ctor;
        exports["dumpNativeAddon"] = Napi::Function::New(env, &NativeAddon::Dump);
        return exports;
    }
    NativeAddon(const Napi::CallbackInfo &info);

    // static NativeAddon* Unwrap(Napi::Value v) {
    //     return Napi::ObjectWrap<NativeAddon>::Unwrap(v.As<Napi::Object>());
    // }

    
    static void Dump(const Napi::CallbackInfo &info) {
        NativeAddon* _this = ::Unwrap<NativeAddon>(info[0]);
        LOG("dump: jsfnref=%p, jsfn=%p", &_this->jsFnRef, &_this->jsFn);
    }


    Napi::Value TryCallByStoredReference(const Napi::CallbackInfo &info);
    Napi::Value TryCallByStoredFunction(const Napi::CallbackInfo &info);
private:
    Napi::FunctionReference jsFnRef;
    Napi::Function jsFn;
};


Napi::Object Register_NativeAddon(Napi::Env env, Napi::Object exports) {
    return NativeAddon::Register(env, exports);    
}


NativeAddon::NativeAddon(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<NativeAddon>(info)
{
    LOG();
    jsFnRef = Napi::Persistent(info[0].As<Napi::Function>());
    jsFn = info[1].As<Napi::Function>();

    LOG("ctro: jsfnref=%p, jsfn=%p", &jsFnRef, &jsFn);
}

Napi::Value NativeAddon::TryCallByStoredReference(const Napi::CallbackInfo &info)
{
    LOG();
    // Napi::Env env = info.Env();
    jsFnRef.Call({});

    return Napi::String::New(info.Env(), "from TryCallByStoredReference");
}

Napi::Value NativeAddon::TryCallByStoredFunction(const Napi::CallbackInfo &info)
{
    LOG();
    // Napi::Env env = info.Env();
    jsFn.Call({});

    return Napi::String::New(info.Env(), "from TryCallByStoredFunction");
}


#include <napi.h>
#include <stdio.h>
#include "napi-helper.hpp"
#include <string_view>
#include "jslog.h"

Napi::Value ArrayBufferArgument(const Napi::CallbackInfo &info)
{
    auto data = AsArray<uint32_t>(info[0]);
    LOGI << fmt::csprintf("data=%p, size=%d", data.data(), data.size());

    JSLOG("data=%p, size=%d", data.data(), data.size());

    for(auto i : data) LOGI << fmt::csprintf("value=%d", i);
    return Napi::Value::From(info.Env(), data.size());
    return Napi::Number::New(info.Env(), data.size());
    return info.Env().Undefined();
}

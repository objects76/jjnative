
#include <napi.h>
#include <stdio.h>
#include "napi-helper.hpp"
#include <string_view>
#include "log.h"

Napi::Value ArrayBufferArgument(const Napi::CallbackInfo &info)
{
    auto data = AsArg<std::u32string_view>(info[0]);
    LOG("data=%p, size=%d", data.data(), data.size());

    for(auto i : data) LOG("value=%d", i);
    return Napi::Value::From(info.Env(), data.size());
    return Napi::Number::New(info.Env(), data.size());
    return info.Env().Undefined();
}

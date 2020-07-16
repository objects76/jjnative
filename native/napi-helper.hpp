
#pragma once
#include <napi.h>
#include <string>
#include <string_view>

template<typename TARGET>
inline TARGET* AsObj(Napi::Value v) { return Napi::ObjectWrap<TARGET>::Unwrap(v.As<Napi::Object>());}



template<typename TARGET>
inline TARGET AsArg(Napi::Value v) { return v.As<TARGET>();}

template<>
inline std::string AsArg<std::string>(Napi::Value v) {
    return v.As<Napi::String>().Utf8Value();
}

template<>
inline std::string_view AsArg<std::string_view>(Napi::Value v) {
    Napi::ArrayBuffer buf = AsArg<Napi::ArrayBuffer>(v);
    return std::string_view((const char*)buf.Data(), buf.ByteLength());
}

template<>
inline std::u16string_view AsArg<std::u16string_view>(Napi::Value v) {
    Napi::ArrayBuffer buf = AsArg<Napi::ArrayBuffer>(v);
    return std::u16string_view((const char16_t*)buf.Data(), buf.ByteLength()/sizeof(char16_t));
}


template<>
inline std::u32string_view AsArg<std::u32string_view>(Napi::Value v) {
    Napi::ArrayBuffer buf = AsArg<Napi::ArrayBuffer>(v);
    return std::u32string_view((const char32_t*)buf.Data(), buf.ByteLength()/sizeof(char32_t));
}


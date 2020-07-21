
#pragma once
#include <napi.h>
#include <string>
#include <string_view>

template<typename TARGET>
inline TARGET* AsObj(Napi::Value v) { return Napi::ObjectWrap<TARGET>::Unwrap(v.As<Napi::Object>());}


template<typename TARGET> inline TARGET AsArg(Napi::Value v) { return v.As<TARGET>();}

template<> inline std::string   AsArg(Napi::Value v) { return v.As<Napi::String>().Utf8Value();}
template<> inline int32_t       AsArg(Napi::Value v) { return v.As<Napi::Number>().Int32Value();}
template<> inline uint32_t      AsArg(Napi::Value v) { return v.As<Napi::Number>().Uint32Value();}
template<> inline float         AsArg(Napi::Value v) { return v.As<Napi::Number>().FloatValue();}
template<> inline bool          AsArg(Napi::Value v) { return v.As<Napi::Boolean>().Value();}

template<> inline Napi::FunctionReference  AsArg(Napi::Value v) { return Napi::Persistent(v.As<Napi::Function>());}


template<typename T> using array_view = std::basic_string_view<T>;

template<typename T>
inline array_view<T> AsArray(Napi::Value v) {
	Napi::ArrayBuffer buf = v.As<Napi::ArrayBuffer>();
    return array_view<T>((const T*)buf.Data(), buf.ByteLength()/sizeof(T));
}
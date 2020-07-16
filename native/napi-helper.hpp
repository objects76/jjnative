
#pragma once
#include <napi.h>

template<typename TARGET>
inline TARGET* Unwrap(Napi::Value v) { return Napi::ObjectWrap<TARGET>::Unwrap(v.As<Napi::Object>());}


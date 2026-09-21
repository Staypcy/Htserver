#ifndef HTSERVER_INCLUDE_HTSERVER_TYPES_H
#define HTSERVER_INCLUDE_HTSERVER_TYPES_H

#include<iostream>
#include<cstdint>

#define INVALID64 (~0ULL)
#define INVALID32 0xFFFFFFFF
#define INVALID16 0xFFFF
#define INVALID8 0xFF

#define MAX_U8 0xFF
#define MAX_U16 0XFFFF
#define MAX_U32 0XFFFFFFFF
#define MAX_U64 (~0ULL)

using u8= uint8_t;
using s8= int8_t;
using u16= uint16_t;
using s16= int16_t;
using u32= uint32_t;
using s32= int32_t;
using u64= uint64_t;
using s64= int64_t;

#endif // HTSERVER_INCLUDE_HTSERVER_TYPES_H

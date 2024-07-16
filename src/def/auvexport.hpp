﻿#pragma once

// CUV_EXPORT
#if defined(CUV_STATICLIB) || defined(CUV_SOURCE)
    #define HV_EXPORT
#elif defined(_MSC_VER)
#if defined(CUV_DYNAMICLIB) || defined(CUV_EXPORTS) || defined(cuv_EXPORTS)
        #define CUV_EXPORT  __declspec(dllexport)
#else
#define CUV_EXPORT  __declspec(dllimport)
#endif
#elif defined(__GNUC__)
    #define HV_EXPORT  __attribute__((visibility("default")))
#else
    #define CUV_EXPORT
#endif

// CUV_DEPRECATED
#if defined(CUV_NO_DEPRECATED)
#define CUV_DEPRECATED
#elif defined(_GNUC_) || defined(__clang__)
#define CUV_DEPRECATED   __attribute__((deprecated))
#elif defined(_MSC_VER)
#define CUV_DEPRECATED   __declspec(deprecated)
#else
#define CUV_DEPRECATED
#endif

// CUV_UNUSED
#if defined(__GNUC__)
    #define CUV_UNUSED   __attribute__((visibility("unused")))
#else
#define CUV_UNUSED
#endif

// @param[IN | OUT | INOUT]
#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

// @field[OPTIONAL | REQUIRED | REPEATED]
#ifndef OPTIONAL
#define OPTIONAL
#endif

#ifndef REQUIRED
#define REQUIRED
#endif

#ifndef REPEATED
#define REPEATED
#endif

#ifdef __cplusplus

#ifndef EXTERN_C
#define EXTERN_C            extern "C"
#endif

#ifndef BEGIN_EXTERN_C
#define BEGIN_EXTERN_C      extern "C" {
#endif

#ifndef END_EXTERN_C
#define END_EXTERN_C        } // extern "C"
#endif

#ifndef BEGIN_NAMESPACE
#define BEGIN_NAMESPACE(ns) namespace ns {
#endif

#ifndef END_NAMESPACE
#define END_NAMESPACE(ns)   } // namespace ns
#endif

#ifndef USING_NAMESPACE
#define USING_NAMESPACE(ns) using namespace ns;
#endif

#ifndef DEFAULT
#define DEFAULT(x)  = x
#endif

#ifndef ENUM
#define ENUM(e)     enum e
#endif

#ifndef STRUCT
#define STRUCT(s)   struct s
#endif

#else

#define EXTERN_C    extern
#define BEGIN_EXTERN_C
#define END_EXTERN_C

#define BEGIN_NAMESPACE(ns)
#define END_NAMESPACE(ns)
#define USING_NAMESPACE(ns)

#ifndef DEFAULT
#define DEFAULT(x)
#endif

#ifndef ENUM
#define ENUM(e)\
typedef enum e e;\
enum e
#endif

#ifndef STRUCT
#define STRUCT(s)\
typedef struct s s;\
struct s
#endif

#endif // __cplusplus

#define BEGIN_NAMESPACE_HV  BEGIN_NAMESPACE(hv)
#define END_NAMESPACE_HV    END_NAMESPACE(hv)
#define USING_NAMESPACE_HV  USING_NAMESPACE(hv)

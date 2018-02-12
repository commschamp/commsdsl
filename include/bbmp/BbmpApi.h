//
// Copyright 2017 (C). Alex Robenko. All rights reserved.
//

#pragma once

#ifdef WIN32

#ifdef BBMP_LIB_EXPORT
#define BBMP_API __declspec(dllexport)
#else // #ifdef BBMP_LIB_EXPORT
#define BBMP_API __declspec(dllimport)
#endif // #ifdef BBMP_LIB_EXPORT

#else // #ifdef WIN32
#define BBMP_API
#endif // #ifdef WIN32

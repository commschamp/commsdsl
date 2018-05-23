//
// Copyright 2017 (C). Alex Robenko. All rights reserved.
//

#pragma once

#ifdef WIN32

#ifdef COMMSDSL_LIB_EXPORT
#define COMMSDSL_API __declspec(dllexport)
#else // #ifdef COMMSDSL_LIB_EXPORT
#define COMMSDSL_API __declspec(dllimport)
#endif // #ifdef COMMSDSL_LIB_EXPORT

#else // #ifdef WIN32
#define COMMSDSL_API
#endif // #ifdef WIN32

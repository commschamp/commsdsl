//
// Copyright 2017 (C). Alex Robenko. All rights reserved.
//

#pragma once

#ifdef WIN32

#ifdef COMMSDSL_LIB_EXPORT
#define COMMSDSL_API __declspec(dllexport)
#define COMMSDSL_EXP_TEMPLATE
#else // #ifdef COMMSDSL_LIB_EXPORT
#define COMMSDSL_API __declspec(dllimport)
#define COMMSDSL_EXP_TEMPLATE extern
#endif // #ifdef COMMSDSL_LIB_EXPORT

#else // #ifdef WIN32
#define COMMSDSL_API
#define COMMSDSL_EXP_TEMPLATE
#endif // #ifdef WIN32

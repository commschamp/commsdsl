//
// Copyright 2019 - 2021 (C). Alex Robenko. All rights reserved.
//

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/// @file
/// Contains version information of the library

#pragma once

/// @brief Major verion of the library
#define COMMSDSL_MAJOR_VERSION 3U

/// @brief Minor verion of the library
#define COMMSDSL_MINOR_VERSION 7U

/// @brief Patch level of the library
#define COMMSDSL_PATCH_VERSION 0U

/// @brief Macro to create numeric version as single unsigned number
#define COMMSDSL_MAKE_VERSION(major_, minor_, patch_) \
    ((static_cast<unsigned>(major_) << 24) | \
     (static_cast<unsigned>(minor_) << 8) | \
     (static_cast<unsigned>(patch_)))

/// @brief Version of the COMMS library as single numeric value
#define COMMSDSL_VERSION COMMSDSL_MAKE_VERSION(COMMSDSL_MAJOR_VERSION, COMMSDSL_MINOR_VERSION, COMMSDSL_PATCH_VERSION)

namespace commsdsl
{

/// @brief Major verion of the library
inline
constexpr unsigned versionMajor()
{
    return COMMSDSL_MAJOR_VERSION;
}

/// @brief Minor verion of the library
inline
constexpr unsigned versionMinor()
{
    return COMMSDSL_MINOR_VERSION;
}

/// @brief Patch level of the library
inline
constexpr unsigned versionPatch()
{
    return COMMSDSL_PATCH_VERSION;
}

/// @brief Create version of the library as single unsigned numeric value.
inline
constexpr unsigned versionCreate(unsigned major, unsigned minor, unsigned patch)
{
    return COMMSDSL_MAKE_VERSION(major, minor, patch);
}

/// @brief Version of the COMMS library as single numeric value
inline
constexpr unsigned version()
{
    return COMMSDSL_VERSION;
}

} // namespace commsdsl

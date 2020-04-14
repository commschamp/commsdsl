//
// Copyright 2018 - 2020 (C). Alex Robenko. All rights reserved.
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

#pragma once

namespace commsdsl
{

enum class Units : unsigned
{
    Unknown,

    // Time
    Nanoseconds,
    Microseconds,
    Milliseconds,
    Seconds,
    Minutes,
    Hours,
    Days,
    Weeks,

    // Distance  
    Nanometers,
    Micrometers,
    Millimeters,
    Centimeters,
    Meters,
    Kilometers,

    // Speed
    NanometersPerSecond,
    MicrometersPerSecond,
    MillimetersPerSecond,
    CentimetersPerSecond,
    MetersPerSecond,
    KilometersPerSecond,
    KilometersPerHour,

    // Frequency
    Hertz,
    KiloHertz,
    MegaHertz,
    GigaHertz,

    // Angle
    Degrees,
    Radians,

    // Electric Current
    Nanoamps,
    Microamps,
    Milliamps,
    Amps,
    Kiloamps,

    // Electric Voltage
    Nanovolts,
    Microvolts,
    Millivolts,
    Volts,
    Kilovolts,

    // Memory Size
    Bytes,
    Kilobytes,
    Megabytes,
    Gigabytes,
    Terabytes,

    NumOfValues
};

} // namespace commsdsl

#pragma once

#include <cstdint>
#include "comms/frame/checksum/BasicSum.h"

namespace test18
{

namespace frame
{

namespace checksum
{

using Sum = comms::frame::checksum::BasicSum<std::uint8_t>;

} // namespace checksum

} // namespace frame

} // namespace test18

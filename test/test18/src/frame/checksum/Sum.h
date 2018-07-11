#pragma once

#include <cstdint>
#include "comms/protocol/checksum/BasicSum.h"

namespace test18
{

namespace frame
{

namespace checksum
{

using Sum = comms::protocol::checksum::BasicSum<std::uint8_t>;

} // namespace checksum

} // namespace frame

} // namespace test18

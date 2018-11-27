#pragma once

#include "comms/protocol/MsgSizeLayer.h"

namespace test18
{

namespace frame
{

namespace layer
{

template <
    typename TField,
    typename TNextLayer,
    typename... TOptions>
using Size = comms::protocol::MsgSizeLayer<TField, TNextLayer>;

} // namespace layer

} // namespace frame

} // namespace test18

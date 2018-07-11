#pragma once

#include "comms/protocol/MsgIdLayer.h"

namespace test18
{

namespace frame
{

namespace layer
{

template <
    typename TField,
    typename TMessage,
    typename TAllMessages,
    typename TNextLayer,
    typename... TOptions>
using Id = comms::protocol::MsgIdLayer<TField, TMessage, TAllMessages, TNextLayer, TOptions...>;

} // namespace layer

} // namespace frame

} // namespace test18

#pragma once

#include "comms/frame/MsgIdLayer.h"

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
using Id = comms::frame::MsgIdLayer<TField, TMessage, TAllMessages, TNextLayer, TOptions...>;

} // namespace layer

} // namespace frame

} // namespace test18

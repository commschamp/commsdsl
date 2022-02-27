//
// Copyright 2019 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "CommsSizeLayer.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"

#include "CommsGenerator.h"

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2new
{

CommsSizeLayer::CommsSizeLayer(CommsGenerator& generator, commsdsl::parse::Layer dslObj, commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

bool CommsSizeLayer::prepareImpl()
{
    return Base::prepareImpl() && CommsBase::prepare();
}

CommsSizeLayer::IncludesList CommsSizeLayer::commsDefIncludesImpl() const
{
    IncludesList result = {
        "comms/protocol/MsgSizeLayer.h"
    };

    return result;
}

} // namespace commsdsl2new

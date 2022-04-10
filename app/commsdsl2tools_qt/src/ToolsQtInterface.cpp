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

#include "ToolsQtInterface.h"

#include "ToolsQtGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2tools_qt
{

ToolsQtInterface::ToolsQtInterface(ToolsQtGenerator& generator, commsdsl::parse::Interface dslObj, commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent)
{
}

std::string ToolsQtInterface::toolsHeaderFilePath() const
{
    auto scope = comms::scopeFor(*this, generator(), false);
    return util::strReplace(scope, "::", "/") + strings::cppHeaderSuffixStr();
}

bool ToolsQtInterface::prepareImpl()
{
    if (!Base::prepareImpl()) {
        return false;
    }

    m_fields = ToolsQtField::toolsTransformFieldsList(fields());
    return true;
}

bool ToolsQtInterface::writeImpl()
{
    // TODO:
    return true;
}


} // namespace commsdsl2tools_qt

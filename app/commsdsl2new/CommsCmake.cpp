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

#include "CommsCmake.h"

#include "CommsGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/comms.h"

#include <fstream>
#include <cassert>

namespace commsdsl2new
{

namespace 
{

using ReplacementMap = commsdsl::gen::util::ReplacementMap;

} // namespace 
    

bool CommsCmake::write(CommsGenerator& generator)
{
    CommsCmake obj(generator);
    return obj.writeInternal();
}

bool CommsCmake::writeInternal() const
{
    static_cast<void>(m_generator);
    auto filePath = 
        commsdsl::gen::util::pathAddElem(
            m_generator.getOutputDir(), commsdsl::gen::strings::cmakeListsFileStr());    

    m_generator.logger().info("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }
    
    // TODO: write

    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

} // namespace commsdsl2new
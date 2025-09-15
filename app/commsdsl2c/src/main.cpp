//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "CGenerator.h"
#include "CProgramOptions.h"

#include <cassert>
#include <iostream>
#include <stdexcept>

int main(int argc, const char* argv[])
{
    try {
        commsdsl2c::CProgramOptions options;
        options.genParse(argc, argv);

        commsdsl2c::CGenerator generator;
        return generator.genExec(options);
    }
    catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        assert(false);
    }

    return -1;
}

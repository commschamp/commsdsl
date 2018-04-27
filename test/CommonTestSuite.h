#pragma once

#include <memory>
#include <vector>
#include <string>

#include "cxxtest/TestSuite.h"
#include "bbmp/bbmp.h"

#ifndef SCHEMAS_DIR
#define SCHEMAS_DIR "."
#endif


class CommonTestSuite
{
public:
    void commonSetUp();
    void commonTearDown();

protected:
    using ProtocolPtr = std::unique_ptr<bbmp::Protocol>;
    using ErrLevelList = std::vector<bbmp::ErrorLevel>;
    ProtocolPtr prepareProtocol(const std::string& schema);

    struct TestStatus
    {
        ErrLevelList m_expErrors;
        bool m_expParseResult = true;
        bool m_expValidateResult = true;
    };

    TestStatus m_status;
};

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <functional>

#include "cxxtest/TestSuite.h"
#include "commsdsl/parse/commsdsl.h"

#ifndef SCHEMAS_DIR
#define SCHEMAS_DIR "."
#endif


class CommonTestSuite
{
public:
    void commonSetUp();
    void commonTearDown();

protected:
    using ProtocolPtr = std::unique_ptr<commsdsl::parse::Protocol>;
    using ErrLevelList = std::vector<commsdsl::parse::ErrorLevel>;
    using PreValidateFunc = std::function<void (commsdsl::parse::Protocol& protocol)>;


    ProtocolPtr prepareProtocol(const std::string& schema, bool enableMultipleSchemas = false);
    ProtocolPtr prepareProtocol(const std::vector<std::string>& schemas, bool enableMultipleSchemas = false);

    struct TestStatus
    {
        ErrLevelList m_expErrors;
        bool m_expParseResult = true;
        bool m_expValidateResult = true;
        PreValidateFunc m_preValidateFunc;
    };

    TestStatus m_status;
};

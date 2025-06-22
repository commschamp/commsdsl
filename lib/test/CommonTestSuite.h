#pragma once

#include <memory>
#include <vector>
#include <string>
#include <functional>

#include "cxxtest/TestSuite.h"
#include "commsdsl/parse/ParseProtocol.h"

#ifndef SCHEMAS_DIR
#define SCHEMAS_DIR "."
#endif


class CommonTestSuite
{
public:
    void commonSetUp();
    void commonTearDown();

protected:
    using ProtocolPtr = std::unique_ptr<commsdsl::parse::ParseProtocol>;
    using ErrLevelList = std::vector<commsdsl::parse::ParseErrorLevel>;
    using PreValidateFunc = std::function<void (commsdsl::parse::ParseProtocol& protocol)>;


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

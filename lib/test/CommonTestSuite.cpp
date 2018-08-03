#include "CommonTestSuite.h"

void CommonTestSuite::commonSetUp()
{
    m_status = TestStatus();
}

void CommonTestSuite::commonTearDown()
{
    TS_ASSERT(m_status.m_expErrors.empty());
}

CommonTestSuite::ProtocolPtr CommonTestSuite::prepareProtocol(const std::string& schema)
{
    static_cast<void>(schema);
    ProtocolPtr protocol(new commsdsl::Protocol);
    protocol->setErrorReportCallback(
        [this](commsdsl::ErrorLevel level, const std::string& msg)
        {
            static const std::string LevelMap[] = {
                "[DEBUG]: ",
                "[INFO]: ",
                "[WARNING]: ",
                "[ERROR]: "
            };
            static const std::size_t LevelMapSize =
                    std::extent<decltype(LevelMap)>::value;

            static_assert(LevelMapSize == commsdsl::ErrorLevel_NumOfValues, "Invalid Map");

            if ((level < static_cast<decltype(level)>(0)) ||
                (commsdsl::ErrorLevel_NumOfValues <= level)) {
                level = commsdsl::ErrorLevel_Error;
            }

            auto errMsg = LevelMap[level] + msg;
            TS_TRACE(errMsg);

            if (m_status.m_expErrors.empty()) {
                TS_ASSERT(level < commsdsl::ErrorLevel_Warning);
                return;
            }

            if (level < commsdsl::ErrorLevel_Warning) {
                return;
            }

            TS_ASSERT_EQUALS(level, m_status.m_expErrors.front());
            m_status.m_expErrors.erase(m_status.m_expErrors.begin());
        });

    bool parseResult = protocol->parse(schema);
    TS_ASSERT_EQUALS(parseResult, m_status.m_expParseResult);

    if (m_status.m_preValidateFunc) {
        m_status.m_preValidateFunc(*protocol);
    }

    bool validateResult = protocol->validate();
    TS_ASSERT_EQUALS(validateResult, m_status.m_expValidateResult);
    return protocol;
}

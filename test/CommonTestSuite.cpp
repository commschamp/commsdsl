#include "CommonTestSuite.h"

void CommonTestSuite::commonSetUp()
{
    m_status = TestStatus();
}

CommonTestSuite::ProtocolPtr CommonTestSuite::prepareProtocol(const std::string& schema)
{
    static_cast<void>(schema);
    ProtocolPtr protocol(new bbmp::Protocol);
    protocol->setErrorReportCallback(
        [this](bbmp::ErrorLevel level, const std::string& msg)
        {
            static const std::string LevelMap[] = {
                "[DEBUG]: ",
                "[INFO]: ",
                "[WARNING]: ",
                "[ERROR]: "
            };
            static const std::size_t LevelMapSize =
                    std::extent<decltype(LevelMap)>::value;

            static_assert(LevelMapSize == bbmp::ErrorLevel_NumOfValues, "Invalid Map");

            if ((level < static_cast<decltype(level)>(0)) ||
                (bbmp::ErrorLevel_NumOfValues <= level)) {
                level = bbmp::ErrorLevel_Error;
            }

            auto errMsg = LevelMap[level] + msg;
            TS_TRACE(errMsg);

            if (m_status.m_expErrors.empty()) {
                TS_ASSERT(level < bbmp::ErrorLevel_Error);
                return;
            }

            if (level < bbmp::ErrorLevel_Warning) {
                return;
            }

            TS_ASSERT_EQUALS(level, m_status.m_expErrors.front());
            m_status.m_expErrors.erase(m_status.m_expErrors.begin());
        });

    bool parseResult = protocol->parse(schema);
    TS_ASSERT_EQUALS(parseResult, m_status.m_expParseResult);

    bool validateResult = protocol->validate();
    TS_ASSERT_EQUALS(validateResult, m_status.m_expValidateResult);
    return protocol;
}

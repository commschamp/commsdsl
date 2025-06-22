#include "CommonTestSuite.h"
#include <algorithm>

void CommonTestSuite::commonSetUp()
{
    m_status = TestStatus();
}

void CommonTestSuite::commonTearDown()
{
    TS_ASSERT(m_status.m_expErrors.empty());
}

CommonTestSuite::ProtocolPtr CommonTestSuite::prepareProtocol(const std::string& schema, bool enableMultipleSchemas)
{
    std::vector<std::string> schemas = {
        schema
    };

    return prepareProtocol(schemas, enableMultipleSchemas);
}

CommonTestSuite::ProtocolPtr CommonTestSuite::prepareProtocol(const std::vector<std::string>& schemas, bool enableMultipleSchemas)
{
    ProtocolPtr protocol(new commsdsl::parse::ParseProtocol);
    protocol->setErrorReportCallback(
        [this](commsdsl::parse::ParseErrorLevel level, const std::string& msg)
        {
            static const std::string LevelMap[] = {
                "[DEBUG]: ",
                "[INFO]: ",
                "[WARNING]: ",
                "[ERROR]: "
            };
            static const std::size_t LevelMapSize =
                    std::extent<decltype(LevelMap)>::value;

            static_assert(LevelMapSize == commsdsl::parse::ParseErrorLevel_NumOfValues, "Invalid Map");

            if ((level < static_cast<decltype(level)>(0)) ||
                (commsdsl::parse::ParseErrorLevel_NumOfValues <= level)) {
                level = commsdsl::parse::ParseErrorLevel_Error;
            }

            auto errMsg = LevelMap[level] + msg;
            TS_TRACE(errMsg);

            if (m_status.m_expErrors.empty()) {
                TS_ASSERT(level < commsdsl::parse::ParseErrorLevel_Warning);
                return;
            }

            if (level < commsdsl::parse::ParseErrorLevel_Warning) {
                return;
            }

            TS_ASSERT_EQUALS(level, m_status.m_expErrors.front());
            m_status.m_expErrors.erase(m_status.m_expErrors.begin());
        });

    protocol->setMultipleSchemasEnabled(enableMultipleSchemas);

    bool parseResult = 
        std::all_of(
            schemas.begin(), schemas.end(),
            [&protocol](auto& s)
            {
                return protocol->parse(s);
            });

    TS_ASSERT_EQUALS(parseResult, m_status.m_expParseResult);

    if (m_status.m_preValidateFunc) {
        m_status.m_preValidateFunc(*protocol);
    }

    bool validateResult = protocol->validate();
    TS_ASSERT_EQUALS(validateResult, m_status.m_expValidateResult);

    auto protSchemas = protocol->schemas();
    TS_ASSERT_LESS_THAN_EQUALS(protSchemas.size(), schemas.size());
    for (auto idx = 0U; idx < schemas.size(); ++idx) {
        auto& s = schemas[idx];
        auto& protSchema = protSchemas[idx];

        auto dotPos = s.find_last_of('.');
        auto slashPos = s.find_last_of('/');
        auto backSlashPos = s.find_last_of('\\');

        if (backSlashPos == std::string::npos) {
            backSlashPos = slashPos;
        }

        if (slashPos == std::string::npos) {
            slashPos = backSlashPos;
        }

        slashPos = std::max(slashPos, backSlashPos);
        TS_ASSERT_DIFFERS(slashPos, std::string::npos);
        TS_ASSERT_LESS_THAN(slashPos, dotPos);
        ++slashPos;
        auto expSchemaName = s.substr(slashPos, dotPos - slashPos);
        TS_ASSERT_EQUALS(protSchema.name(), expSchemaName);     
    }
    return protocol;
}

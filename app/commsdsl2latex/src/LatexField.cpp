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

#include "LatexField.h"

#include "LatexGenerator.h"

#include "commsdsl/gen/GenBitfieldField.h"
#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2latex
{

namespace 
{

const std::size_t InvalidLength = std::numeric_limits<std::size_t>::max();    

bool isLengthInBits(const LatexField& f)
{
    auto* parent = f.latexGenField().genGetParent();
    assert(parent != nullptr);
    if (parent == nullptr) {
        return false;
    }

    if (parent->genElemType() != LatexField::GenElem::Type_Field) {
        return false;
    }

    auto* parentField = static_cast<const LatexField::GenField*>(parent);
    return parentField->genParseObj().parseKind() == commsdsl::parse::ParseField::ParseKind::Bitfield;
}

bool isLengthInBits(const LatexField::LatexFieldsList& fields)
{
    return
        std::any_of(
            fields.begin(), fields.end(),
            [](auto* f)
            {
                assert(f != nullptr);
                return isLengthInBits(*f);
            });    
}

} // namespace 
    

LatexField::LatexField(const commsdsl::gen::GenField& field) :
    m_genField(field)
{
}

LatexField::~LatexField() = default;

std::string LatexField::latexRelFilePath() const
{
    assert(comms::genIsGlobalField(m_genField));
    if (!m_genField.genIsReferenced()) {
        // Not referenced fields do not need to be written
        return strings::genEmptyString();
    }    
    
    auto& latexGenerator = LatexGenerator::latexCast(m_genField.genGenerator());
    return latexGenerator.latexRelPathFor(m_genField) + strings::genLatexSuffixStr();
}

std::string LatexField::latexTitle() const
{
    auto name = LatexGenerator::latexEscDisplayName(m_genField.genParseObj().parseDisplayName(), m_genField.genParseObj().parseName());
    if (comms::genIsGlobalField(m_genField)) {
        return "Field \"" + name + "\"";
    }

    return "Member Field \"" + name + "\"";
}

std::string LatexField::latexDoc() const
{
    static const std::string Templ = 
            "#^#SECTION#$#"
            "\\label{#^#LABEL#$#}\n\n"
            "#^#DESCRIPTION#$#\n"
            "#^#PREPEND#$#\n"
            "#^#INFO#$#\n"
            "#^#DETAILS#$#\n"
            "#^#APPEND#$#\n"
        ;

    util::GenReplacementMap repl = {
        {"SECTION", latexSection()},
        {"LABEL", LatexGenerator::latexLabelId(m_genField)},
        {"DESCRIPTION", util::genStrMakeMultiline(m_genField.genParseObj().parseDescription())},
        {"INFO", latexInfoDetails()},
        {"DETAILS", latexExtraDetailsImpl()},
    };

    LatexGenerator::latexEnsureNewLineBreak(repl["DESCRIPTION"]);    

    do {
        if (!comms::genIsGlobalField(m_genField)) {
            break;
        }

        auto& latexGenerator = LatexGenerator::latexCast(m_genField.genGenerator());
        auto relFilePath = latexRelFilePath();
        auto prependFileName = relFilePath + strings::genPrependFileSuffixStr();
        auto appendFileName = relFilePath + strings::genAppendFileSuffixStr();


        repl["PREPEND"] = util::genReadFileContents(latexGenerator.latexInputCodePathForFile(appendFileName));
        repl["APPEND"] = util::genReadFileContents(latexGenerator.latexInputCodePathForFile(prependFileName));

        if (!latexGenerator.latexHasCodeInjectionComments()) {
            break;
        };           

        if (repl["PREPEND"].empty()) {
            repl["PREPEND"] = latexGenerator.latexCodeInjectCommentPrefix() + "Prepend to details with \"" + prependFileName + "\".";
        } 
                        
        if (repl["APPEND"].empty()) {
            repl["APPEND"] = latexGenerator.latexCodeInjectCommentPrefix() + "Append to file with \"" + appendFileName + "\".";
        }                

    } while (false);

    
    return util::genProcessTemplate(Templ, repl);
}

std::string LatexField::latexRefLabelId() const
{
    return latexRefLabelIdImpl();
}

LatexField::LatexFieldsList LatexField::latexTransformFieldsList(const GenFieldsList& fields)
{
    LatexFieldsList result;
    result.reserve(fields.size());
    for (auto& fPtr : fields) {
        assert(fPtr);

        auto* latexField = 
            const_cast<LatexField*>(
                dynamic_cast<const LatexField*>(fPtr.get()));

        assert(latexField != nullptr);
        result.push_back(latexField);
    }

    return result;
}

std::string LatexField::latexMembersDetails(const LatexFieldsList& latexFields)
{
    bool inBits = isLengthInBits(latexFields);

    std::string units = "Bytes";
    if (inBits) {
        units = "Bits";
    }

    util::GenStringsList lines;
    util::GenStringsList fields;    
    std::size_t offset = 0;
    for (auto* f : latexFields) {
        auto& genField = f->latexGenField();
        auto parseObj = genField.genParseObj();
        auto details = f->latexDoc();
        std::string nameStr = LatexGenerator::latexEscDisplayName(parseObj.parseDisplayName(), parseObj.parseName());
        if (!details.empty()) {
            nameStr = "\\hyperref[" + f->latexRefLabelId() + "]{" + nameStr + "}";
            fields.push_back(details);
        }

        if (f->latexIsOptional()) {
            nameStr += " (optional)";
        }

        auto minLength = parseObj.parseMinLength();
        auto maxLength = parseObj.parseMaxLength();

        if (inBits) {
            minLength = parseObj.parseBitLength();
            maxLength = minLength;
        }

        auto lengthStr = std::to_string(minLength);
        do {
            if (maxLength == minLength) {
                break;
            }

            if (maxLength == InvalidLength) {
                lengthStr += '+';
                break;
            }

            lengthStr += " - " + std::to_string(maxLength);
        } while (false);

        std::string offsetStr;
        do {
            if (offset == InvalidLength) {
                offsetStr = "<variable>";
                break;
            }

            offsetStr = std::to_string(offset);
            if (minLength == maxLength) {
                offset += minLength;
                break;
            }

            offset = InvalidLength;
        } while (false);

        lines.push_back(offsetStr + " & " + lengthStr + " & " + nameStr);
    }

    static const std::string Templ = 
        "\\fbox{%\n"
        "\\begin{tabular}{c|c|c}\n"
        "\\textbf{Offset (#^#UNITS#$#)} & \\textbf{Length (#^#UNITS#$#)}& \\textbf{Name}\\\\\n"
        "\\hline\n"
        "\\hline\n"
        "#^#LINES#$#\n"
        "\\end{tabular}\n"
        "}\n"
        "\\smallskip\n"
        "#^#DETAILS#$#\n"
        "\n"
        ;    

    util::GenReplacementMap repl = {
        {"LINES", util::genStrListToString(lines, " \\\\\\hline\n", " \\\\")},
        {"DETAILS", util::genStrListToString(fields, "\n", "\n")},
        {"UNITS", units},
    };

    return util::genProcessTemplate(Templ, repl);      
}

bool LatexField::latexWrite() const
{
    if (!comms::genIsGlobalField(m_genField)) {
        // Skip write for non-global fields,
        // The code generation will be driven by other means        
        return true;
    }

    if (!m_genField.genIsReferenced()) {
        // Not referenced fields do not need to be written
        m_genField.genGenerator().genLogger().genDebug(
            "Skipping file generation for non-referenced field \"" + m_genField.genParseObj().parseExternalRef() + "\".");
        return true;
    }

    auto& latexGenerator = LatexGenerator::latexCast(m_genField.genGenerator());
    auto filePath = util::genPathAddElem(latexGenerator.genGetOutputDir(), latexRelFilePath());

    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!latexGenerator.genCreateDirectory(dirPath)) {
        return false;
    }    

    latexGenerator.genLogger().genInfo("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        latexGenerator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    do {
        auto replaceFileName = latexRelFilePath() + strings::genReplaceFileSuffixStr();
        auto replaceContents = util::genReadFileContents(latexGenerator.latexInputCodePathForFile(replaceFileName));
        if (!replaceContents.empty()) {
            stream << replaceContents;
            break;
        }

        static const std::string Templ = 
            "#^#GENERATED#$#\n"
            "#^#REPLACE_COMMENT#$#\n"
            "#^#DOC#$#\n"
            ;
        util::GenReplacementMap repl = {
            {"GENERATED", LatexGenerator::latexFileGeneratedComment()},
            {"DOC", latexDoc()},
        };

        if (latexGenerator.latexHasCodeInjectionComments()) {
            repl["REPLACE_COMMENT"] = 
                latexGenerator.latexCodeInjectCommentPrefix() + "Replace the whole file with \"" + replaceFileName + "\".";
        };         

        stream << util::genProcessTemplate(Templ, repl, true) << std::endl;
    } while (false);

    stream.flush();
    if (!stream.good()) {
        latexGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }    

    return true;    
}

std::string LatexField::latexSection() const
{
    static const std::string Templ = 
        "#^#REPLACE_COMMENT#$#\n"
        "#^#SECTION#$#{#^#TITLE#$#}\n"
        ;

    auto& latexGenerator = LatexGenerator::latexCast(m_genField.genGenerator());
    util::GenReplacementMap repl = {
        {"SECTION", LatexGenerator::latexSectionDirective(m_genField)},
    };

    if (comms::genIsGlobalField(m_genField)) {
        auto titleFileName = latexRelFilePath() + strings::genTitleFileSuffixStr();
        repl["TITLE"] = util::genReadFileContents(latexGenerator.latexInputCodePathForFile(titleFileName));
        
        if (latexGenerator.latexHasCodeInjectionComments()) {
            repl["REPLACE_COMMENT"] = 
                latexGenerator.latexCodeInjectCommentPrefix() + "Replace the title value with contents of \"" + titleFileName + "\".";
        };      

    }

    if (repl["TITLE"].empty()) {
        repl["TITLE"] = latexTitle();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string LatexField::latexRefLabelIdImpl() const
{
    return LatexGenerator::latexLabelId(m_genField);
}

std::string LatexField::latexInfoDetailsImpl() const
{
    return strings::genEmptyString();
}

std::string LatexField::latexExtraDetailsImpl() const
{
    return strings::genEmptyString();
}

const std::string& LatexField::fieldKindImpl() const
{
    static const std::string Map[] = {
        /* Int */ "Integral",
        /* Enum */ "Enumeration",
        /* Set */ "Bits",
        /* Float */ "Floating Point",
        /* Bitfield */ "Bitfield",
        /* Bundle */ "Bundle (Composite)",
        /* String */ "String",
        /* Data */ "Raw Data",
        /* List */ "List",
        /* Ref */ strings::genEmptyString(), // Must be overriden
        /* Optional */ strings::genEmptyString(), // Must be overriden
        /* Variant */ "Variant", 
    };
    static const std::size_t MapSize = std::extent_v<decltype(Map)>;

    auto idx = static_cast<unsigned>(m_genField.genParseObj().parseKind());
    assert(idx < MapSize);
    if (MapSize <= idx) {
        return strings::genEmptyString();
    }

    return Map[idx];
}

bool LatexField::latexIsOptionalImpl() const
{
    return false;
}

std::string LatexField::latexSignedInfo(ParseIntField::ParseType value)
{
    static const std::string* Map[] = {
        /* Int8 */ &strings::genYesStr(),
        /* Uint8 */ &strings::genNoStr(),
        /* Int16 */ &strings::genYesStr(),
        /* Uint16 */ &strings::genNoStr(),
        /* Int32 */ &strings::genYesStr(),
        /* Uint32 */ &strings::genNoStr(),
        /* Int64 */ &strings::genYesStr(),
        /* Uint64 */ &strings::genNoStr(),
        /* Intvar */ &strings::genYesStr(),
        /* Uintvar */ &strings::genNoStr(),
    };
    static const std::size_t MapSize = std::extent_v<decltype(Map)>;
    static_assert(MapSize == static_cast<unsigned>(ParseIntField::ParseType::NumOfValues));

    auto idx = static_cast<unsigned>(value);
    assert(idx < MapSize);
    return "\\textbf{Signed} & " + *(Map[idx]);
}

std::string LatexField::latexEndianInfo(commsdsl::parse::ParseEndian value)
{
    static const std::string* Map[] = {
        /* ParseEndian_Little */ &strings::genLittleStr(),
        /* ParseEndian_Big */ &strings::genBigStr(),
    };
    static const std::size_t MapSize = std::extent_v<decltype(Map)>;
    static_assert(MapSize == static_cast<unsigned>(commsdsl::parse::ParseEndian_NumOfValues));

    auto idx = static_cast<unsigned>(value);
    assert(idx < MapSize);
    return "\\textbf{Endian} & " + *(Map[idx]);
}

std::string LatexField::latexUnitsInfo(commsdsl::parse::ParseUnits value)
{
    if ((value == commsdsl::parse::ParseUnits::Unknown) ||
        (commsdsl::parse::ParseUnits::NumOfValues <= value)) {
        return strings::genEmptyString();
    }

    static const std::string Map[] = {
        /* Unknown */ std::string(),

        // Time
        /* Nanoseconds */ "Nanoseconds",
        /* Microseconds */ "Microseconds",
        /* Milliseconds */ "Milliseconds",
        /* Seconds */ "Seconds",
        /* Minutes */ "Minutes",
        /* Hours */ "Hours",
        /* Days */ "Days",
        /* Weeks */ "Weeks",

        // Distance  
        /* Nanometers */ "Nanometers",
        /* Micrometers */ "Micromiters",
        /* Millimeters */ "Millimeters",
        /* Centimeters */ "Centimeters",
        /* Meters */ "Meters",
        /* Kilometers */ "Kilometers",

        // Speed
        /* NanometersPerSecond */ "Nanometers Per Second",
        /* MicrometersPerSecond */ "Micrometers Per Second",
        /* MillimetersPerSecond */ "Millimeters Per Second",
        /* CentimetersPerSecond */ "Centimeters Per Second",
        /* MetersPerSecond */ "Meters Per Second",
        /* KilometersPerSecond */ "Kilometers Per Second",
        /* KilometersPerHour */ "Kilometers Per Hour",

        // Frequency
        /* Hertz */ "Hertz",
        /* KiloHertz */ "Kilohertz",
        /* MegaHertz */ "Megahertz",
        /* GigaHertz */ "Gigahertz",

        // Angle
        /* Degrees */ "Degrees",
        /* Radians */ "Radians",

        // Electric Current
        /* Nanoamps */ "Nano Amperes",
        /* Microamps */ "Micro Amperes",
        /* Milliamps */ "Milli Amperes",
        /* Amps */ "Amperes",
        /* Kiloamps */ "Kilo Amperes",

        // Electric Voltage
        /* Nanovolts */ "Nano Volts",
        /* Microvolts */ "Micro Volts",
        /* Millivolts */ "Milli Volts",
        /* Volts */ "Volts",
        /* Kilovolts */ "Killo Volts",

        // Memory Size
        /* Bytes */ "Bytes",
        /* Kilobytes */ "Kilobytes",
        /* Megabytes */ "Megabytes",
        /* Gigabytes */ "Kigabytes",
        /* Terabytes */ "Terabytes",
    };
    static const std::size_t MapSize = std::extent_v<decltype(Map)>;
    static_assert(MapSize == static_cast<unsigned>(commsdsl::parse::ParseUnits::NumOfValues));

    auto idx = static_cast<unsigned>(value);
    assert(idx < MapSize);
    return "\\textbf{Units} & " + (Map[idx]);
}

std::string LatexField::latexInfoDetails() const
{
    static const std::string Templ = 
        "\\subsubparagraph{Details}\n"
        "\\label{#^#LABEL#$#}\n\n"
        "\\fbox{%\n"
        "\\begin{tabular}{l|p{5cm}}\n"
        "#^#LINES#$##^#SEP#$#\n"
        "#^#EXTRA#$#\n"
        "\\end{tabular}\n"
        "}\n"
        "\\smallskip\n"
        "\n"
        ;

    util::GenStringsList lines;    
    
    
    auto parseObj = m_genField.genParseObj();
    auto makeNumericStr = 
        [](auto val)
        {
            std::stringstream stream;
            stream << val << " (0x" << std::setw(2) << std::setfill('0') << std::hex << val << ")";
            return stream.str();
        };

    do{
        lines.push_back("\\textbf{Field Kind} & " + fieldKindImpl());
    } while (false); 
    
    do {
        auto minLength = parseObj.parseMinLength();
        auto maxLength = parseObj.parseMaxLength();

        bool inBits = isLengthInBits(*this);
        std::string units = " Bytes";
        if (inBits) {
            minLength = parseObj.parseBitLength();
            maxLength = minLength;
            units = " Bits";
        }

        if (minLength == maxLength) {
            lines.push_back("\\textbf{Fixed Length} & " + std::to_string(minLength) + units);
            break;
        }

        if (maxLength != InvalidLength) {
            lines.push_back("\\textbf{Variable Length} & " + std::to_string(minLength) + " - " + std::to_string(maxLength) + units);
            break;
        }

        lines.push_back("\\textbf{Variable Length} & " + std::to_string(minLength) + "+" + units);
    } while (false);

    do {
        auto sinceVersion = parseObj.parseSinceVersion();
        if (sinceVersion == 0) {
            break;
        }

        lines.push_back("\\textbf{Introduced In Version} &" + makeNumericStr(sinceVersion));
    } while (false);

    do {
        auto deprecatedSince = parseObj.parseDeprecatedSince();
        if (deprecatedSince == commsdsl::parse::ParseProtocol::parseNotYetDeprecated()) {
            break;
        }

        lines.push_back("\\textbf{Deprecated In Version} &" + makeNumericStr(deprecatedSince));
    } while (false);      

    util::GenReplacementMap repl = {
        {"LABEL", LatexGenerator::latexLabelId(m_genField) + "_details"},
        {"LINES", util::genStrListToString(lines, " \\\\\\hline\n", " \\\\")},
        {"EXTRA", latexInfoDetailsImpl()},
    };

    if (!repl["EXTRA"].empty()) {
        repl["SEP"] = "\\hline";
    }

    return util::genProcessTemplate(Templ, repl);
}

bool LatexField::latexIsOptional() const
{
    auto& dslObj = m_genField.genParseObj();
    if (m_genField.genGenerator().genIsElementOptional(dslObj.parseSinceVersion(), dslObj.parseDeprecatedSince(), dslObj.parseIsDeprecatedRemoved())) {
        return true;
    }

    auto* parent = m_genField.genGetParent();
    assert(parent != nullptr);
    if (comms::genSinceVersionOf(*parent) < dslObj.parseSinceVersion()) {
        return true;
    }

    if ((dslObj.parseDeprecatedSince() < commsdsl::parse::ParseProtocol::parseNotYetDeprecated()) &&
        (dslObj.parseIsDeprecatedRemoved())) {
        return true;
    }

    return latexIsOptionalImpl();
}

} // namespace commsdsl2latex

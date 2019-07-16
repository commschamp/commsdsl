//
// Copyright 2019 (C). Alex Robenko. All rights reserved.
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

#include "Test.h"

#include <fstream>

#include <boost/filesystem.hpp>

#include "Generator.h"
#include "common.h"
#include "EnumField.h"
#include "IntField.h"

namespace bf = boost::filesystem;

namespace commsdsl2comms
{

bool Test::write(Generator& generator)
{
    Test obj(generator);
    return obj.writeInputTest();
}

bool Test::writeInputTest() const
{
    auto dir = m_generator.testDir();
    if (dir.empty()) {
        return false;
    }

    auto testName = 
        m_generator.mainNamespace() + '_' + common::inputTestStr() + common::srcSuffix();;

    bf::path filePath(dir);
    filePath /= testName;

    std::string filePathStr(filePath.string());

    m_generator.logger().info("Generating " + filePathStr);
    std::ofstream stream(filePathStr);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePathStr + "\" for writing.");
        return false;
    }

    common::ReplacementMap replacements;
    
    std::string idType;
    auto* idField = m_generator.getMessageIdField();
    if ((idField != nullptr) && (idField->kind() == commsdsl::Field::Kind::Enum)) {
        auto* enumMsgIdField = static_cast<const EnumField*>(idField);
        if (enumMsgIdField->isUnsignedUnderlyingType()) {
            idType = "std::uintmax_t";
        }
        else {
            idType = "std::intmax_t";
        }

        auto hexWidth = enumMsgIdField->hexWidth();
        if (hexWidth != 0U) {
            replacements.insert(std::make_pair("BEFORE_ID", " << \"0x\" << std::hex << std::setfill('0') << std::setw(" + std::to_string(hexWidth) + ")"));
            replacements.insert(std::make_pair("AFTER_ID", " << std::dec"));
        }
    }
    else if ((idField != nullptr) && (idField->kind() == commsdsl::Field::Kind::Int)) {
        auto* intMsgIdField = static_cast<const IntField*>(idField);
        if (intMsgIdField->isUnsignedType()) {
            idType = "std::uintmax_t";
        }
        else {
            idType = "std::intmax_t";
        }
    }
    else {
        idType = m_generator.mainNamespace() + "::" + common::msgIdEnumNameStr();
    }

    replacements.insert(std::make_pair("ID_TYPE", idType));

    static const std::string Template = 
        "#include <iostream>\n"
        "#include <fstream>\n"
        "#include <cstring>\n"
        "#include <cstdlib>\n"
        "#include <array>\n"
        "#include <vector>\n"
        "#include <iomanip>\n\n"
        "#include \"comms/fields.h\"\n"
        "#include \"comms/iterator.h\"\n\n"
        "#define QUOTES_(x_) #x_\n"
        "#define QUOTES(x_) QUOTES_(x_)\n\n"
        "#ifndef INTERFACE_HEADER\n"
        "#error \"Interface header needs to be defined\"\n"
        "#endif\n\n"
        "#ifndef INTERFACE\n"
        "#error \"Interface type needs to be defined\"\n"
        "#endif\n\n"
        "#ifndef FRAME_HEADER\n"
        "#error \"Frame header needs to be defined\"\n"
        "#endif\n\n"
        "#ifndef FRAME\n"
        "#error \"Frame type needs to be defined\"\n"
        "#endif\n\n"
        "#ifndef OPTIONS_HEADER\n"
        "#error \"Options header needs to be defined\"\n"
        "#endif\n\n"
        "#ifndef OPTIONS\n"
        "#error \"Options type needs to be defined\"\n"
        "#endif\n\n"
        "#ifndef INPUT_MESSAGES_HEADER\n"
        "#error \"Input messages header needs to be defined\"\n"
        "#endif\n\n"
        "#ifndef INPUT_MESSAGES\n"
        "#error \"Input messages type needs to be defined\"\n"
        "#endif\n\n"
        "#include QUOTES(INTERFACE_HEADER)\n"
        "#include QUOTES(FRAME_HEADER)\n"
        "#include QUOTES(OPTIONS_HEADER)\n"
        "#include QUOTES(INPUT_MESSAGES_HEADER)\n\n"
        "namespace\n"
        "{\n\n"
        "struct Handler;\n"
        "using Message = \n"
        "    INTERFACE<\n"
        "        comms::option::app::ReadIterator<const char*>,\n"
        "        comms::option::app::WriteIterator<char*>,\n"
        "        comms::option::app::LengthInfoInterface,\n"
        "        comms::option::app::Handler<Handler>\n"
        "    >;\n\n"
        "using AppOptions = OPTIONS;\n"
        "using InputMessages = INPUT_MESSAGES<Message, AppOptions>;\n"
        "using Frame = FRAME<Message, InputMessages, AppOptions>;\n\n"
        "Frame frame;\n\n"
        "void printIndent(unsigned indent)\n"
        "{\n"
        "    while (0U < indent) {\n"
        "        static const std::string IndStr(\"    \");\n"
        "        std::cout << IndStr;\n"
        "        --indent;\n"
        "    }\n"
        "}\n\n"
        "// Forward declaration of function used to print variant field\n"
        "template <typename TField>\n"
        "void printVariantField(const TField& field, unsigned indent);\n\n"
        "class FieldPrinter\n"
        "{\n"
        "public:\n"
        "    explicit FieldPrinter(unsigned indent) : m_indent(indent) {}\n\n"
        "    template <typename TField>\n"
        "    void operator()(const TField& field) const\n"
        "    {\n"
        "        using FieldType = typename std::decay<decltype(field)>::type;\n"
        "        using Tag = FieldTag<FieldType>;\n"
        "        printIndent(m_indent);\n"
        "        std::cout << field.name() << \" : \";\n"
        "        printFieldValue(field, Tag());\n"
        "        std::cout << \'\\n\';\n"
        "    }\n\n"
        "private: \n"
        "    struct CastTag {};\n"
        "    struct AsIsTag {};\n"
        "    struct IntElementTag {};\n"
        "    struct FieldElementTag {};\n"
        "    struct EnumFieldTag {};\n"
        "    struct BitmaskFieldTag {};\n"
        "    struct GenericFieldTag {};\n\n"
        "    template <typename TField>\n"
        "    using FieldTag = \n"
        "        typename std::conditional<\n"
        "            comms::field::isEnumValue<TField>(),\n"
        "            EnumFieldTag,\n"
        "            typename std::conditional<\n"
        "                comms::field::isBitmaskValue<TField>(),\n"
        "                BitmaskFieldTag,\n"
        "                GenericFieldTag\n"
        "            >::type\n"
        "        >::type;\n\n"
        "    template <typename TFieldBase, typename T, typename... TOptions>\n"
        "    static void printFieldValue(const comms::field::IntValue<TFieldBase, T, TOptions...>& field, GenericFieldTag)\n"
        "    {\n"
        "        printIntValue(field.value());\n"
        "    }\n\n"
        "    template <typename TField>\n"
        "    static void printFieldValue(const TField& field, EnumFieldTag)\n"
        "    {\n"
        "        using FieldType = typename std::decay<decltype(field)>::type;\n"
        "        using ValueType = typename FieldType::ValueType;\n"
        "        using UnderlyingType = typename std::underlying_type<ValueType>::type;\n\n"
        "        auto* name = field.valueName(field.value());\n"
        "        if (name == nullptr) {\n"
        "            printIntValue(static_cast<UnderlyingType>(field.value()));\n"
        "            return;\n"
        "        }\n\n"
        "        std::cout << name << \" (\";\n"
        "        printIntValue(static_cast<UnderlyingType>(field.value()));\n"
        "        std::cout << \')\';\n"
        "    }\n\n"
        "    template <typename TField>\n"
        "    void printFieldValue(const TField& field, BitmaskFieldTag) const\n"
        "    {\n"
        "        using FieldType = typename std::decay<decltype(field)>::type;\n"
        "        std::cout << std::hex << \"0x\" << static_cast<std::uintmax_t>(field.value());\n"
        "        for (auto idx = 0U; idx < FieldType::BitIdx_numOfValues; ++idx) {\n"
        "            auto bitIdx = static_cast<typename FieldType::BitIdx>(idx);\n"
        "            auto* name = field.bitName(bitIdx);\n"
        "            if (name == nullptr) {\n"
        "                continue;\n"
        "            }\n\n"
        "            std::cout << \'\\n\';\n"
        "            printIndent(m_indent + 1);\n"
        "            std::cout << name << \" : \" << std::boolalpha << field.getBitValue(bitIdx);\n"
        "        }\n"
        "    }\n\n"
        "    template <typename TFieldBase, typename TMembers, typename... TOptions>\n"
        "    void printFieldValue(const comms::field::Bitfield<TFieldBase, TMembers, TOptions...>& field, GenericFieldTag) const\n"
        "    {\n"
        "        std::cout << \'\\n\';\n"
        "        comms::util::tupleForEach(field.value(), FieldPrinter(m_indent + 1));\n"
        "    }\n\n"
        "    template <typename TFieldBase, typename TMembers, typename... TOptions>\n"
        "    void printFieldValue(const comms::field::Bundle<TFieldBase, TMembers, TOptions...>& field, GenericFieldTag) const\n"
        "    {\n"
        "        std::cout << \'\\n\';\n"
        "        comms::util::tupleForEach(field.value(), FieldPrinter(m_indent + 1));\n"
        "    }\n\n"
        "    template <typename TFieldBase, typename T, typename... TOptions>\n"
        "    static void printFieldValue(const comms::field::FloatValue<TFieldBase, T, TOptions...>& field, GenericFieldTag)\n"
        "    {\n"
        "        std::cout << field.value();\n"
        "    }\n\n"
        "    template <typename TFieldBase, typename... TOptions>\n"
        "    static void printFieldValue(const comms::field::String<TFieldBase, TOptions...>& field, GenericFieldTag)\n"
        "    {\n"
        "        std::cout << std::string(field.value().begin(), field.value().end());\n"
        "    }\n\n"
        "    template <typename TFieldBase, typename TElement, typename... TOptions>\n"
        "    void printFieldValue(const comms::field::ArrayList<TFieldBase, TElement, TOptions...>& field, GenericFieldTag) const\n"
        "    {\n"
        "        using FieldType = typename std::decay<decltype(field)>::type;\n"
        "        using Tag = \n"
        "            typename std::conditional<\n"
        "                std::is_integral<typename FieldType::ElementType>::value,\n"
        "                IntElementTag,\n"
        "                FieldElementTag\n"
        "            >::type;\n\n"
        "        printArrayData(field.value(), Tag());\n"
        "    }\n\n"
        "    template <typename TField, typename... TOptions>\n"
        "    void printFieldValue(const comms::field::Optional<TField, TOptions...>& field, GenericFieldTag) const\n"
        "    {\n"
        "        if (field.isMissing()) {\n"
        "            std::cout << \"<missing>\";\n"
        "            return;\n"
        "        }\n\n"
        "        if (!field.doesExist()) {\n"
        "            assert(!\"Should not happen\");\n"
        "            exit(-1);\n"
        "        }\n\n"
        "        std::cout << \"<exists>\\n\";\n"
        "        FieldPrinter printer(m_indent + 1);\n"
        "        printer(field.field());\n"
        "    }\n\n"
        "    template <typename TFieldBase, typename TMembers, typename... TOptions>\n"
        "    void printFieldValue(const comms::field::Variant<TFieldBase, TMembers, TOptions...>& field, GenericFieldTag) const\n"
        "    {\n"
        "        if (!field.currentFieldValid()) {\n"
        "            std::cout << \"<none>\";\n"
        "            return;\n"
        "        }\n\n"
        "        std::cout << \"(\" << field.currentField() << \")\\n\";\n"
        "        printVariantField(field, m_indent + 1);\n"
        "    }\n\n"
        "    template <typename T>\n"
        "    static void printIntValue(T value)\n"
        "    {\n"
        "        using Tag = \n"
        "            typename std::conditional<\n"
        "                sizeof(T) == 1U,\n"
        "                CastTag,\n"
        "                AsIsTag\n"
        "            >::type;\n\n"
        "        return printIntValue(value, Tag());\n"
        "    }\n\n"
        "    template <typename T>\n"
        "    static void printIntValue(T value, CastTag)\n"
        "    {\n"
        "        using CastType = \n"
        "            typename comms::util::SizeToType<sizeof(int), std::is_signed<T>::value>::Type;\n"
        "        std::cout << static_cast<CastType>(value);\n"
        "    }\n\n"
        "    template <typename T>\n"
        "    static void printIntValue(T value, AsIsTag)\n"
        "    {\n"
        "        std::cout << value;\n"
        "    }\n\n"
        "    template <typename TVec>\n"
        "    static void printArrayData(const TVec& data, IntElementTag)\n"
        "    {\n"
        "        for (auto v : data) {\n"
        "            std::cout << std::setfill(\'0\') << std::setw(2) << std::hex << \n"
        "                static_cast<unsigned>(v) << \" \";\n"
        "        }\n"
        "    }\n\n"
        "    template <typename TVec>\n"
        "    void printArrayData(const TVec& data, FieldElementTag) const\n"
        "    {\n"
        "        if (data.empty()) {\n"
        "            return;\n"
        "        }\n\n"
        "        std::cout << \'\\n\';\n"
        "        FieldPrinter printer(m_indent + 1);\n"
        "        for (auto& f : data) {\n"
        "            printer(f);\n"
        "        }\n"
        "    }\n\n"
        "    unsigned m_indent = 0U;\n"
        "};\n\n"
        "class VariantFieldPrinter\n"
        "{\n"
        "public:\n"
        "    explicit VariantFieldPrinter(unsigned indent) : m_printer(indent) {}\n\n"
        "    template <std::size_t TIdx, typename TField>\n"
        "    void operator()(const TField& field)\n"
        "    {\n"
        "        m_printer(field);\n"
        "    }\n"
        "private:\n"
        "    FieldPrinter m_printer;\n"
        "};\n\n"
        "template <typename TField>\n"
        "void printVariantField(const TField& field, unsigned indent)\n"
        "{\n"
        "    field.currentFieldExec(VariantFieldPrinter(indent));\n"
        "}\n\n"
        "struct Handler\n"
        "{\n"
        "    template <typename TMsg>\n"
        "    void handle(TMsg& msg)\n"
        "    {\n"
        "        std::cout << \'\\n\' << msg.doName() << \" (\"#^#BEFORE_ID#$# << static_cast<#^#ID_TYPE#$#>(msg.doGetId())#^#AFTER_ID#$# << \"):\\n\";\n"
        "        comms::util::tupleForEach(msg.fields(), FieldPrinter(1));\n"
        "        std::cout << std::endl;\n\n"
        "        // Check write is correct\n"
        "        std::size_t len = frame.length(msg);\n"
        "        assert(0U < len);\n"
        "        std::unique_ptr<char []> outBuf(new char[len]);\n"
        "        auto writeIter = &(outBuf[0]);\n"
        "        auto es = frame.write(msg, writeIter, len);\n"
        "        if (es != comms::ErrorStatus::Success) {\n"
        "            std::cerr << \"ERROR: Failed to write\" << std::endl;\n"
        "            assert(!\"Should not happen\");\n"
        "            exit(-1);\n"
        "        }\n\n"
        "        if (writeIter != (&outBuf[0] + len)) {\n"
        "            std::cerr << \"ERROR: Unexpected pos of write iterator\" << std::endl;\n"
        "            assert(!\"Should not happen\");\n"
        "            exit(-1);\n"            
        "        }\n"
        "    }\n"
        "};\n\n"
        "Handler handler; // Handler object\n\n"
        "// Receives input buffer and its size and returns number of consumed bytes\n"
        "std::size_t processInput(const char* buf, std::size_t len)\n"
        "{\n"
        "    std::size_t consumed = 0U;\n"
        "    // Processing loop\n"
        "    while (consumed < len) {\n"
        "        // Smart pointer to the message object.\n"
        "        Frame::MsgPtr msgPtr;\n"
        "        // Type of the message interface class\n"
        "        using MsgType = Frame::MsgPtr::element_type;\n\n"
        "        // Get the iterator for reading\n"
        "        auto begIter = comms::readIteratorFor<MsgType>(buf + consumed);\n"
        "        auto iter = begIter;\n\n"
        "        // Do the read\n"
        "        auto remLen = len - consumed;\n"
        "        auto es = frame.read(msgPtr, iter, remLen);\n"
        "        if (es == comms::ErrorStatus::NotEnoughData) {\n"
        "            break; // Not enough data in the buffer, stop processing\n"
        "        }\n\n"
        "        if (es == comms::ErrorStatus::ProtocolError) {\n"
        "            // Something is not right with the data, remove one character and try again\n"
        "            ++consumed;\n"
        "            continue;\n"
        "        }\n\n"
        "        // The iterator for reading has been advanced, update the difference\n"
        "        auto diff = static_cast<std::size_t>(std::distance(begIter, iter));\n"
        "        assert(diff <= remLen);\n"
        "        consumed += diff;\n"
        "        assert(consumed <= len);\n\n"
        "        if (es == comms::ErrorStatus::Success) {\n"
        "            assert(msgPtr); // If read is successful, msgPtr is expected to hold a valid pointer\n"
        "            msgPtr->dispatch(handler); // Dispatch message for handling\n"
        "        }\n"
        "    }\n"
        "    // Report how many bytes have been consumed from the buffer\n"
        "    return consumed;\n"
        "}\n\n"
        "} // namespace\n\n"
        "int main(int argc, const char* argv[])\n"
        "{\n"
        "    static_cast<void>(argc);\n"
        "    static_cast<void>(argv);\n\n"
        "    auto* openResult = std::freopen(nullptr, \"rb\", stdin);\n"
        "    static_cast<void>(openResult);\n\n"
        "    if(std::ferror(stdin)) {\n"
        "        std::cerr << \"Failed to open stdin\" << std::endl;\n"
        "        return -1;\n"
        "    }\n\n"
        "    std::array<char, 1024> buf;\n"
        "    std::vector<char> input;\n"
        "    while (true) {\n"
        "        std::size_t len = std::fread(buf.data(), sizeof(buf[0]), buf.size(), stdin);\n\n"
        "        if(std::ferror(stdin)) {\n"
        "            if (std::feof(stdin)) {\n"
        "                return 0;\n"
        "            }\n\n"
        "            std::cerr << \"Some error\" << std::endl;\n"
        "            return -1;\n"
        "        }\n\n"
        "        if (len == 0U) {\n"
        "            return 0;\n"
        "        }\n\n"
        "        input.insert(input.end(), buf.data(), buf.data() + len); // append to vector\n\n"
        "        auto consumed = processInput(&input[0], input.size());\n"
        "        input.erase(input.begin(), input.begin() + consumed);\n"
        "    }\n"
        "    return 0;\n"
        "}\n\n";

    auto str = common::processTemplate(Template, replacements);
    stream << str;

    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePathStr + "\".");
        return false;
    }

    return true;
}

} // namespace commsdsl2comms

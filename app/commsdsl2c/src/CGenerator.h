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

#pragma once

#include "commsdsl/gen/GenGenerator.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2c 
{

class CGenerator final : public commsdsl::gen::GenGenerator
{
    using Base = commsdsl::gen::GenGenerator;

public:
    using GenStringsList = commsdsl::gen::util::GenStringsList;
    using GenElem = commsdsl::gen::GenElem;
    using GenSchemaPtr = commsdsl::gen::GenSchemaPtr;
    using GenNamespacePtr = commsdsl::gen::GenNamespacePtr;
    using GenMessagePtr = commsdsl::gen::GenMessagePtr;
    using GenFramePtr = commsdsl::gen::GenFramePtr;
    using GenProgramOptions = commsdsl::gen::GenProgramOptions;
    using GenGenerator = commsdsl::gen::GenGenerator;
    using GenFieldPtr = commsdsl::gen::GenFieldPtr;

    static const std::string& cFileGeneratedComment();

    static CGenerator& cCast(GenGenerator& generator)
    {
        return static_cast<CGenerator&>(generator);
    }

    static const CGenerator& cCast(const GenGenerator& generator)
    {
        return static_cast<const CGenerator&>(generator);
    }    

    std::string cRelHeaderFor(const commsdsl::gen::GenElem& elem) const;
    std::string cAbsHeaderFor(const commsdsl::gen::GenElem& elem) const;
    std::string cRelSourceFor(const commsdsl::gen::GenElem& elem) const;
    std::string cAbsSourceFor(const commsdsl::gen::GenElem& elem) const;  
    
    std::string cInputAbsHeaderFor(const commsdsl::gen::GenElem& elem) const;
    std::string cInputAbsSourceFor(const commsdsl::gen::GenElem& elem) const;

    static std::string cScopeToName(const std::string& scope);    
    static const std::string& cCppGuardBegin();
    static const std::string& cCppGuardEnd();

    const std::string& cNamesPrefix() const;
    const GenStringsList& cProtocolOptions() const;

protected:
    virtual bool genPrepareImpl() override;
    virtual bool genWriteImpl() override;    

    // virtual GenSchemaPtr genCreateSchemaImpl(ParseSchema parseObj, GenElem* parent) override;
    // virtual GenNamespacePtr genCreateNamespaceImpl(ParseNamespace parseObj, GenElem* parent) override;
    // virtual GenMessagePtr genCreateMessageImpl(ParseMessage parseObj, GenElem* parent) override;
    // virtual GenFramePtr genCreateFrameImpl(ParseFrame parseObj, GenElem* parent) override;

    virtual GenFieldPtr genCreateIntFieldImpl(ParseField parseObj, GenElem* parent) override;
    virtual GenFieldPtr genCreateEnumFieldImpl(ParseField parseObj, GenElem* parent) override;
    virtual GenFieldPtr genCreateSetFieldImpl(ParseField parseObj, GenElem* parent) override;
    virtual GenFieldPtr genCreateFloatFieldImpl(ParseField parseObj, GenElem* parent) override;
    virtual GenFieldPtr genCreateBitfieldFieldImpl(ParseField parseObj, GenElem* parent) override;
    virtual GenFieldPtr genCreateBundleFieldImpl(ParseField parseObj, GenElem* parent) override;
    virtual GenFieldPtr genCreateStringFieldImpl(ParseField parseObj, GenElem* parent) override;
    virtual GenFieldPtr genCreateDataFieldImpl(ParseField parseObj, GenElem* parent) override;
    virtual GenFieldPtr genCreateListFieldImpl(ParseField parseObj, GenElem* parent) override;
    virtual GenFieldPtr genCreateRefFieldImpl(ParseField parseObj, GenElem* parent) override;
    virtual GenFieldPtr genCreateOptionalFieldImpl(ParseField parseObj, GenElem* parent) override;
    virtual GenFieldPtr genCreateVariantFieldImpl(ParseField parseObj, GenElem* parent) override;

    virtual OptsProcessResult genProcessOptionsImpl(const GenProgramOptions& options) override;

private:
    bool cWriteExtraFilesInternal() const;
    void cSetNamesPrefixInternal(const std::string& value);
    void cSetCommsOptions(const std::string& value);

    bool cPrepareNamesPrefixInternal();
    bool cPrepareCommsOptionsInternal();

    std::string m_namesPrefix;    
    GenStringsList m_commsOptions;
};

} // namespace commsdsl2c

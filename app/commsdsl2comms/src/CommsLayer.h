//
// Copyright 2019 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
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

#include "CommsField.h"

#include "commsdsl/gen/GenLayer.h"
#include "commsdsl/gen/util.h"

#include <string>
#include <vector>

namespace commsdsl2comms
{

class CommsLayer
{
public:
    using GenStringsList = commsdsl::gen::util::GenStringsList;

    using GenLayer = commsdsl::gen::GenLayer;

    using CommsIncludesList = GenStringsList;
    using CommsLayersList = std::vector<CommsLayer*>;

    explicit CommsLayer(GenLayer& layer);
    virtual ~CommsLayer();

    [[nodiscard]] static CommsLayer* commsCast(GenLayer* obj)
    {
        return dynamic_cast<CommsLayer*>(obj);
    }

    [[nodiscard]] static const CommsLayer* commsCast(const GenLayer* obj)
    {
        return dynamic_cast<const CommsLayer*>(obj);
    }

    [[nodiscard]] bool commsPrepare();

    [[nodiscard]] CommsIncludesList commsCommonIncludes() const;
    [[nodiscard]] std::string commsCommonCode() const;
    [[nodiscard]] CommsIncludesList commsDefIncludes() const;
    [[nodiscard]] std::string commsDefType(const CommsLayer* prevLayer, bool& hasInputMessages) const;
    [[nodiscard]] bool commsIsCustomizable() const;
    void commsSetForcedPseudoField();
    void commsSetForcedFailOnInvalidField();

    [[nodiscard]] std::string commsDefaultOptions() const;
    [[nodiscard]] std::string commsDataViewDefaultOptions() const;
    [[nodiscard]] std::string commsBareMetalDefaultOptions() const;
    [[nodiscard]] std::string commsMsgFactoryDefaultOptions() const;

    [[nodiscard]] const GenLayer& commsGenLayer() const
    {
        return m_genLayer;
    }

    [[nodiscard]] const CommsField* commsExternalField() const
    {
        return m_commsExternalField;
    }

    [[nodiscard]] const CommsField* commsMemberField() const
    {
        return m_commsMemberField;
    }

    [[nodiscard]] CommsField* commsMemberField()
    {
        return m_commsMemberField;
    }

protected:
    [[nodiscard]] virtual CommsIncludesList commsCommonIncludesImpl() const;
    [[nodiscard]] virtual CommsIncludesList commsDefIncludesImpl() const;
    [[nodiscard]] virtual std::string commsDefBaseTypeImpl(const std::string& prevName) const;
    [[nodiscard]] virtual bool commsDefHasInputMessagesImpl() const;
    [[nodiscard]] virtual GenStringsList commsDefExtraOptsImpl() const;
    [[nodiscard]] virtual bool commsIsCustomizableImpl() const;
    [[nodiscard]] virtual GenStringsList commsExtraDataViewDefaultOptionsImpl() const;
    [[nodiscard]] virtual GenStringsList commsExtraBareMetalDefaultOptionsImpl() const;
    [[nodiscard]] virtual GenStringsList commsExtraMsgFactoryDefaultOptionsImpl() const;
    [[nodiscard]] virtual std::string commsCustomDefMembersCodeImpl() const;
    [[nodiscard]] virtual std::string commsCustomFieldOptsImpl() const;
    [[nodiscard]] virtual std::string commsCustomFieldDataViewOptsImpl() const;
    [[nodiscard]] virtual std::string commsCustomFieldBareMetalOptsImpl() const;
    [[nodiscard]] virtual std::string commsExtraMemberFieldsDefsImpl() const;
    [[nodiscard]] virtual std::string commsExtraMemberFieldsCommonCodeImpl() const;

    [[nodiscard]] std::string commsDefFieldType() const;
    [[nodiscard]] std::string commsDefExtraOpts() const;
    [[nodiscard]] static std::string commsMsgFactoryAliasInOptions(const commsdsl::gen::GenElem* parent);

private:
    using CommsFieldOptsFunc = std::string (CommsField::*)() const;
    using CommsExtraLayerOptsFunc = GenStringsList (CommsLayer::*)() const;

    [[nodiscard]] std::string commsDefMembersCodeInternal() const;
    [[nodiscard]] std::string commsDefDocInternal() const;
    [[nodiscard]] std::string commsCustomizationOptionsInternal(
        CommsFieldOptsFunc fieldOptsFunc,
        CommsExtraLayerOptsFunc extraLayerOptsFunc,
        bool hasBase,
        const std::string& customFieldOpts) const;

    [[nodiscard]] GenStringsList commsExtraDataViewDefaultOptionsInternal() const;
    [[nodiscard]] GenStringsList commsExtraBareMetalDefaultOptionsInternal() const;
    [[nodiscard]] GenStringsList commsExtraMsgFactoryDefaultOptionsInternal() const;

    GenLayer& m_genLayer;
    CommsField* m_commsExternalField = nullptr;
    CommsField* m_commsMemberField = nullptr;

    bool m_forcedPseudoField = false;
    bool m_forcedFailedOnInvalidField = false;
};

} // namespace commsdsl2comms

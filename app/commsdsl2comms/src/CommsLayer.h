//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
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

    static CommsLayer* commsCast(GenLayer* layer)
    {
        return dynamic_cast<CommsLayer*>(layer);
    }

    static const CommsLayer* commsCast(const GenLayer* layer)
    {
        return dynamic_cast<const CommsLayer*>(layer);
    }    

    bool commsPrepare();

    CommsIncludesList commsCommonIncludes() const;
    std::string commsCommonCode() const;
    CommsIncludesList commsDefIncludes() const;
    std::string commsDefType(const CommsLayer* prevLayer, bool& hasInputMessages) const;
    bool commsIsCustomizable() const;
    void commsSetForcedPseudoField();
    void commsSetForcedFailOnInvalidField();

    std::string commsDefaultOptions() const;
    std::string commsDataViewDefaultOptions() const;
    std::string commsBareMetalDefaultOptions() const;
    std::string commsMsgFactoryDefaultOptions() const;

    const GenLayer& commsGenLayer() const
    {
        return m_genLayer;
    }

    const CommsField* commsExternalField() const
    {
        return m_commsExternalField;
    }

    const CommsField* commsMemberField() const
    {
        return m_commsMemberField;
    }   

    CommsField* commsMemberField()
    {
        return m_commsMemberField;
    }     


protected:
    virtual CommsIncludesList commsDefIncludesImpl() const;
    virtual std::string commsDefBaseTypeImpl(const std::string& prevName) const;
    virtual bool commsDefHasInputMessagesImpl() const;
    virtual GenStringsList commsDefExtraOptsImpl() const; 
    virtual bool commsIsCustomizableImpl() const;
    virtual GenStringsList commsExtraDataViewDefaultOptionsImpl() const;
    virtual GenStringsList commsExtraBareMetalDefaultOptionsImpl() const;
    virtual GenStringsList commsExtraMsgFactoryDefaultOptionsImpl() const;
    virtual std::string commsCustomDefMembersCodeImpl() const;
    virtual std::string commsCustomFieldOptsImpl() const;
    virtual std::string commsCustomFieldDataViewOptsImpl() const;
    virtual std::string commsCustomFieldBareMetalOptsImpl() const;

    std::string commsDefFieldType() const;
    std::string commsDefExtraOpts() const;
    static std::string commsMsgFactoryAliasInOptions(const commsdsl::gen::GenElem* parent);

private:
    using CommsFieldOptsFunc = std::string (CommsField::*)() const;
    using CommsExtraLayerOptsFunc = GenStringsList (CommsLayer::*)() const;

    std::string commsDefMembersCodeInternal() const;
    std::string commsDefDocInternal() const;
    std::string commsCustomizationOptionsInternal(
        CommsFieldOptsFunc fieldOptsFunc, 
        CommsExtraLayerOptsFunc extraLayerOptsFunc,
        bool hasBase,
        const std::string& customFieldOpts) const;  

    GenStringsList commsExtraDataViewDefaultOptionsInternal() const;
    GenStringsList commsExtraBareMetalDefaultOptionsInternal() const;
    GenStringsList commsExtraMsgFactoryDefaultOptionsInternal() const;
    
    GenLayer& m_genLayer;
    CommsField* m_commsExternalField = nullptr;
    CommsField* m_commsMemberField = nullptr;

    bool m_forcedPseudoField = false;
    bool m_forcedFailedOnInvalidField = false;
};

} // namespace commsdsl2comms

//
// Copyright 2019 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/Layer.h"
#include "commsdsl/gen/util.h"

#include "CommsField.h"

#include <string>
#include <vector>

namespace commsdsl2comms
{

class CommsLayer
{
public:
    using StringsList = commsdsl::gen::util::StringsList;
    using IncludesList = StringsList;
    using CommsLayersList = std::vector<CommsLayer*>;

    explicit CommsLayer(commsdsl::gen::Layer& layer);
    virtual ~CommsLayer();

    static CommsLayer* cast(commsdsl::gen::Layer* layer)
    {
        return dynamic_cast<CommsLayer*>(layer);
    }

    static const CommsLayer* cast(const commsdsl::gen::Layer* layer)
    {
        return dynamic_cast<const CommsLayer*>(layer);
    }    

    bool commsPrepare();

    IncludesList commsCommonIncludes() const;
    std::string commsCommonCode() const;
    IncludesList commsDefIncludes() const;
    std::string commsDefType(const CommsLayer* prevLayer, bool& hasInputMessages) const;
    bool commsIsCustomizable() const;
    void commsSetForcedPseudoField();
    void commsSetForcedFailOnInvalidField();

    std::string commsDefaultOptions() const;
    std::string commsDataViewDefaultOptions() const;
    std::string commsBareMetalDefaultOptions() const;
    std::string commsMsgFactoryDefaultOptions() const;

    const commsdsl::gen::Layer& layer() const
    {
        return m_layer;
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
    virtual IncludesList commsDefIncludesImpl() const;
    virtual std::string commsDefBaseTypeImpl(const std::string& prevName) const;
    virtual bool commsDefHasInputMessagesImpl() const;
    virtual StringsList commsDefExtraOptsImpl() const; 
    virtual bool commsIsCustomizableImpl() const;
    virtual StringsList commsExtraDataViewDefaultOptionsImpl() const;
    virtual StringsList commsExtraBareMetalDefaultOptionsImpl() const;
    virtual StringsList commsExtraMsgFactoryDefaultOptionsImpl() const;
    virtual std::string commsCustomDefMembersCodeImpl() const;
    virtual std::string commsCustomFieldOptsImpl() const;
    virtual std::string commsCustomFieldDataViewOptsImpl() const;
    virtual std::string commsCustomFieldBareMetalOptsImpl() const;

    std::string commsDefFieldType() const;
    std::string commsDefExtraOpts() const;

private:
    using FieldOptsFunc = std::string (CommsField::*)() const;
    using ExtraLayerOptsFunc = StringsList (CommsLayer::*)() const;

    std::string commsDefMembersCodeInternal() const;
    std::string commsDefDocInternal() const;
    std::string commsCustomizationOptionsInternal(
        FieldOptsFunc fieldOptsFunc, 
        ExtraLayerOptsFunc extraLayerOptsFunc,
        bool hasBase,
        const std::string& customFieldOpts) const;  

    StringsList commsExtraDataViewDefaultOptionsInternal() const;
    StringsList commsExtraBareMetalDefaultOptionsInternal() const;
    StringsList commsExtraMsgFactoryDefaultOptionsInternal() const;
    
    commsdsl::gen::Layer& m_layer;
    CommsField* m_commsExternalField = nullptr;
    CommsField* m_commsMemberField = nullptr;

    bool m_forcedPseudoField = false;
    bool m_forcedFailedOnInvalidField = false;
};

} // namespace commsdsl2comms

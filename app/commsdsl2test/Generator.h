#pragma once

#include "commsdsl/gen/Generator.h"

namespace commsdsl2test 
{

class Generator final : public commsdsl::gen::Generator
{
public:
    static const std::string& fileGeneratedComment();

protected:
    virtual bool writeImpl() override;    
};

} // namespace commsdsl2test

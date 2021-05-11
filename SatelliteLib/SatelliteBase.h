#pragma once
#include <Error.h>
#include <ISatellite.h>
#include <stack>
#include <vector>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace Sat
{

class SatelliteBase : public ISatellite
{
public:

    virtual void load() = 0;
    virtual void unload() = 0;
    virtual Error checkout_info(char const* p_targetPath, CheckoutInfo& p_out_checkoutInfo) = 0;

    virtual Error relay(char const * p_originPath, char const* p_query, CStringWrapper& p_out_string) override;

    virtual ~SatelliteBase(){}

protected:
    Error resolve_expression(std::vector<std::string> const & p_tokens, std::string const & p_originalExpression, nlohmann::json& p_currentDict,
        std::stack<std::string>& p_fileStack, CStringWrapper& p_out_string);
    Error evaluate_terms(std::filesystem::path const & p_absolute_prefix, std::string const & p_terms, std::string & p_out_evaluated);
};

}
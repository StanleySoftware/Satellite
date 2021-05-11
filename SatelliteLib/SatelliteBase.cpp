#include "SatelliteBase.h"
#include <string>
#include <fstream>
#include <sstream>

#include <GitProxy.h>
#include <Utility.h>
#include <CheckoutInfo.h>

namespace Sat
{

namespace filesys = std::filesystem;

static constexpr char const * c_satelliteFileName = "satellite.json";

Error SatelliteBase::relay(char const * p_originPath, char const* p_query, CStringWrapper& p_out_string)
{
//Working copy root
CheckoutInfo checkout{};
Error err{};
err = checkout_info(p_originPath, checkout);
if (err.errorCode())
{
    return err;
}

if (!checkout.m_isCheckout)
{
    err.m_errorType = ErrorType::NOTACHECKOUT;
    err.m_errorMessage = str_format("Satellite: Path '%s' is not part of a checkout.", p_originPath);
    return err;
}

if (!checkout.m_checkoutRoot.get())
{
    err.m_errorType = ErrorType::BARECHECKOUT;
    err.m_errorMessage = str_format("Satellite: Path '%s' is part of a bare checkout.", p_originPath);
    return err;
}

// read the JSON
filesys::path satelliteFile = filesys::path(checkout.m_checkoutRoot.get()).append(c_satelliteFileName);
std::ifstream fileStream(satelliteFile);
nlohmann::json satelliteJson;
try
{
    fileStream >> satelliteJson;
}
catch (nlohmann::json::exception& json_err)
{
    err.m_errorType = ErrorType::JSONPARSE;
    err.m_errorMessage = str_format("Satellite: While attempting to parse '%s', hit the following error: '%s'", satelliteFile.c_str(), json_err.what());
    return err;
}

std::vector<std::string> tokens = str_split(p_query, ":");
std::stack<std::string> fileStack{};
fileStack.emplace(satelliteFile.string());
CStringWrapper reply{};
err = resolve_expression(tokens, p_query, satelliteJson, fileStack, reply);
if(err.errorCode() == 0)
{
    p_out_string = std::move(reply);
}

return err;
}

Error SatelliteBase::evaluate_terms(std::filesystem::path const & p_absolute_prefix, std::string const & p_terms, std::string & p_out_evaluated)
{
    std::vector<std::string> seperatedArgs{};
    Error err = separate_arg_string(p_terms, seperatedArgs);
    if (err.errorCode())
    {
        return err;
    }

    std::string result{};
    char const delimiter = ' ';

    for (std::string& term : seperatedArgs)
    {
        std::filesystem::path termAsPath(term);
        if(termAsPath.is_relative())
        {
            term = (p_absolute_prefix / termAsPath).string();
        }
        result += term;
        result += delimiter;
    }

    if (result.length() > 0u)
    {
        //Shave the extra delimiter
        result.pop_back();
    }

    p_out_evaluated = result;
    return Error{};
}

Error SatelliteBase::resolve_expression(std::vector<std::string> const & p_tokens, std::string const & p_originalExpression, nlohmann::json& p_currentDict,
    std::stack<std::string>& p_fileStack, CStringWrapper& p_out_string)
{
    Error err{};
    if(p_tokens.size() == 0)
    {
        err.m_errorType = ErrorType::EMPTYQUERY;
        err.m_errorMessage = str_format("Satellite: ResolveExpression called with no tokens. Invalid expression: \'%s\'", p_originalExpression.c_str());
        return err;
    }

    nlohmann::json satelliteJson = p_currentDict;

    for(std::string const & token : p_tokens)
    {
        std::string& fileNameTop = p_fileStack.top();
        std::string const & leadingKey = token;
        nlohmann::json& leadingValue = satelliteJson[leadingKey];

        filesys::path currentDir(fileNameTop);
        if(!filesys::is_directory(currentDir))
        {
            currentDir = currentDir.parent_path();
        }

        if (leadingValue.is_null())
        {
            err.m_errorType = ErrorType::EMPTYVALUE;
            err.m_errorMessage = str_format("Satellite: Term \'%s\' in query \'%s\' evaluates to null in file \'%s\'.", leadingKey.c_str(), p_originalExpression.c_str(), fileNameTop.c_str());
            return err;
        }

        std::string evalTerms{};
        Error err = evaluate_terms(currentDir, leadingValue, evalTerms);
        if (err.errorCode())
        {
            return err;
        }

        if (&token == &p_tokens.back())
        {
            //Last term, so return the looked up value

            size_t stringSize = evalTerms.size() + 1u;
            CStringWrapper returnVal(stringSize);
            std::copy(evalTerms.c_str(), evalTerms.c_str() + stringSize, returnVal.get());
            p_out_string = std::move(returnVal);
            return err;
        }
        else
        {
            //Not the last term, keep resolving

            filesys::path termsAsPath(evalTerms);
            if (!termsAsPath.has_extension() || (termsAsPath.extension() != ".json"))
            {
                err.m_errorType = ErrorType::FILENOTJSON;
                err.m_errorMessage = str_format("Satellite: File \'%s\' is not a .json file, resolved from lookup of key \'%s\' in file \'%s\'.", evalTerms.c_str(), leadingKey.c_str(), fileNameTop.c_str());
                return err;
            }

            if (!filesys::exists(termsAsPath))
            {
                err.m_errorType = ErrorType::FILEDOESNOTEXIST;
                err.m_errorMessage = str_format("Satellite: File \'%s\' does not exist, resolved from lookup of key \'%s\' in file \'%s\'.", evalTerms.c_str(), leadingKey.c_str(), fileNameTop.c_str());
                return err;
            }

            //Set satelliteJson to this json
            std::ifstream fileStream(termsAsPath);
            satelliteJson.clear();
            try
            {
                fileStream >> satelliteJson;
            }
            catch (nlohmann::json::exception& json_err)
            {
                err.m_errorType = ErrorType::JSONPARSE;
                err.m_errorMessage = str_format("Satellite: While attempting to parse '%s', hit the following error: '%s'", termsAsPath.c_str(), json_err.what());
                return err;
            }

            //Add this json to the filestack for the next iteration
            p_fileStack.push(termsAsPath.string());
        }

    }

    return err;
}

}
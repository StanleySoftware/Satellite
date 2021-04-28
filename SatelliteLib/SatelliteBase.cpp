#include "SatelliteBase.h"
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>

#include <GitProxy.h>
#include <Utility.h>
#include <CheckoutInfo.h>

#include <nlohmann/json.hpp>

namespace Sat
{

namespace filesys = std::filesystem;

static constexpr char const * c_satelliteFileName = "satellite.json";

std::unique_ptr<char const[]> SatelliteBase::relay(char const * p_originPath, char const* p_query)
{
//Working copy root
CheckoutInfo checkout = checkout_info(p_originPath);
if (!checkout.m_isCheckout)
{
    m_lastError.m_errorType = ErrorType::NOTACHECKOUT;
    m_lastError.m_errorMessage = str_format("Satellite: Path '%s' is not part of a checkout.", p_originPath);
    return nullptr;
}

if (!checkout.m_checkoutRoot)
{
    m_lastError.m_errorType = ErrorType::BARECHECKOUT;
    m_lastError.m_errorMessage = str_format("Satellite: Path '%s' is part of a bare checkout.", p_originPath);
    return nullptr;
}

// read the JSON
filesys::path satelliteFile = filesys::path(p_originPath) / c_satelliteFileName;
std::ifstream fileStream(satelliteFile);
nlohmann::json satelliteJson;
try
{
    fileStream >> satelliteJson;
}
catch (nlohmann::json::exception& err)
{
    m_lastError.m_errorType = ErrorType::JSONPARSE;
    m_lastError.m_errorMessage = str_format("Satellite: While attempting to parse '%s', hit the following error: '%s'", satelliteFile, err.what());
    return nullptr;
}

std::string buffer;
std::stringstream ss(p_query);
std::vector<std::string> tokens;

while (std::getline(ss, buffer, ':'))
{
    tokens.push_back(buffer);
}

std::stack<std::string> fileStack{};
fileStack.emplace(satelliteFile.string());
//return ResolveExpression(tokens, p_query, satelliteJson, fileStack);

return nullptr;
}

Error SatelliteBase::get_last_error()
{
    return m_lastError;
}

Error SatelliteBase::resolve_expression(std::vector<std::string>& p_tokens, std::string p_originalExpression, nlohmann::json& p_currentDict, std::stack<std::string>& p_fileStack)
{
    return Error();
}

}
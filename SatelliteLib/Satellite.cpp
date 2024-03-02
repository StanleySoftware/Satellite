#include "Satellite.h"
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

#include <Utility.h>
#include <WorkspaceInfo.h>
#include <optional>

namespace Sat
{

namespace filesys = std::filesystem;

#define SATELLITE_FILENAME "satellite"
#define SATELLITE_FILE_EXTENSION ".json"
static constexpr char const * c_satelliteFileName = SATELLITE_FILENAME SATELLITE_FILE_EXTENSION;

Error Satellite::relay(char const * p_originPath, char const* p_query, CStringWrapper& p_out_string)
{
    //Working copy root
    WorkspaceInfo Workspace{};
    Error err{};
    err = workspace_info(p_originPath, Workspace);
    if (err.errorCode())
    {
        return err;
    }

    if (!Workspace.m_isWorkspace)
    {
        err.m_errorType = ErrorType::NOTAWORKSPACE;
        err.m_errorMessage = str_format("Satellite: Path '%s' is not part of a Workspace.", p_originPath);
        return err;
    }

    if (!Workspace.m_workspaceRoot.get())
    {
        err.m_errorType = ErrorType::BAREWORKSPACE;
        err.m_errorMessage = str_format("Satellite: Path '%s' is part of a bare Workspace.", p_originPath);
        return err;
    }

    // read the JSON
    filesys::path satelliteFile = filesys::path(Workspace.m_workspaceRoot.get());
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

Error Satellite::evaluate_terms(std::filesystem::path const & p_absolute_prefix, std::string const & p_terms, std::string & p_out_evaluated)
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

Error Satellite::resolve_expression(std::vector<std::string> const & p_tokens, std::string const & p_originalExpression, nlohmann::json& p_currentDict,
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

Error Satellite::workspace_info(char const* p_targetPath, WorkspaceInfo& p_out_WorkspaceInfo)
{
    namespace fs = std::filesystem;
    Error err{};
    const fs::path expectedFileName{c_satelliteFileName};

    std::optional<fs::path> uppermostMatchingFile;
    fs::path inspectedPath{p_targetPath};
    inspectedPath = inspectedPath.make_preferred();

    auto getPathIfValid = [expectedFileName](const fs::path& pathToInspect, std::optional<fs::path>& outPath)
    {
        if(!fs::exists(pathToInspect))
        {
            return false;
        }

        if(fs::is_regular_file(pathToInspect) && (pathToInspect.filename() == expectedFileName))
        {
            outPath = pathToInspect;
            return true;
        }

        return false;
    };

    if(!fs::exists(inspectedPath))
    {
        err.m_errorType = ErrorType::INVALIDPATH;
        err.m_errorMessage = str_format("Satellite: Path \'%s\' is not recognised by the file system.", p_targetPath);
        return err;
    }

    getPathIfValid(inspectedPath, uppermostMatchingFile);

    fs::path parentPath = inspectedPath.parent_path();
    while(inspectedPath != parentPath)
    {
        const fs::path potentialPath = inspectedPath.append(SATELLITE_FILENAME).replace_extension(SATELLITE_FILE_EXTENSION);

        getPathIfValid(potentialPath, uppermostMatchingFile);

        inspectedPath = parentPath;
        parentPath = inspectedPath.parent_path();
    }

    if(uppermostMatchingFile.has_value())
    {
        p_out_WorkspaceInfo.m_isWorkspace = true;
        p_out_WorkspaceInfo.m_workspaceRoot = Sat::str_format(uppermostMatchingFile->string());
    }

    return Error();
}

}
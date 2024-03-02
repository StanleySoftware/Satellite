#pragma once
#include <string>
#include <memory>
#include "Error.h"
#include "WorkspaceInfo.h"

namespace Sat
{

class ISatellite
{
public:
    virtual void load() = 0;
    virtual void unload() = 0;

    /// @brief Returns VCS WorkspaceInfo for the path.
    /// @param p_targetPath - The path to check.
    virtual Error workspace_info(char const* p_targetPath, WorkspaceInfo& p_out_WorkspaceInfo) = 0;
 
    virtual Error relay(char const * p_originPath, char const* p_query, CStringWrapper& p_out_string) = 0;

    virtual ~ISatellite(){};
};

}
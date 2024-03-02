#pragma once

#include <Utility.h>

namespace Sat
{

/// @brief Holds information about a Workspace
struct WorkspaceInfo
{
	WorkspaceInfo(){}

	WorkspaceInfo(bool p_isWorkspace, CStringWrapper&& p_workspaceRoot)
		: m_isWorkspace(p_isWorkspace)
		, m_workspaceRoot(std::move(p_workspaceRoot))
	{}

	/// @brief True if the path is determined to be part of a version controlled Workspace
	bool m_isWorkspace = false;
	/// @brief If m_isWorkspace is true then this is:
	/// - nullptr if the Workspace is bare
	/// - The path to the root of the Workspace if it is not bare
	/// If m_isWorkspace is false then this is nullptr.
	CStringWrapper m_workspaceRoot{};
};

}
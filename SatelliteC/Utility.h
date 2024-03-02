#pragma once

#define NOMINMAX
#include "SatelliteC.h"

#include <vector>

#include <Error.h>
#include <CStringWrapper.h>
#include <WorkspaceInfo.h>

namespace SatC
{

using CStringWrapper = Sat::CStringWrapper;

inline std::vector<CStringWrapper*>* s_string_table = nullptr;
inline constexpr StrHnd c_string_table_no_init = std::numeric_limits<StrHnd>::max();
inline constexpr char const * c_no_init_message = "Satellite has not been initialised. Please call init() before calling any other API functions.";

StrHnd to_interface_type(CStringWrapper && p_cstring)
{
	if (s_string_table)
	{
		CStringWrapper * allocCString = new CStringWrapper(std::move(p_cstring));
		s_string_table->push_back(allocCString);
		StrHnd hnd = static_cast<StrHnd>(s_string_table->size() - 1u);
		return hnd;
	}
	return c_string_table_no_init;
}

Error to_interface_type(Sat::Error && p_error)
{
	Error err{};
	err.m_errorType = static_cast<int32_t>(p_error.m_errorType);
	err.m_errorMessage = to_interface_type(std::move(p_error.m_errorMessage));
	return err;
}

WorkspaceInfo to_interface_type(Sat::WorkspaceInfo && p_Workspaceinfo)
{
	WorkspaceInfo ci{};
	ci.m_isWorkspace = p_Workspaceinfo.m_isWorkspace;
	ci.m_workspaceRoot = to_interface_type(std::move(p_Workspaceinfo.m_workspaceRoot));
	return ci;
}

template<typename ... Args>
Error create_error(Sat::ErrorType p_errType, char const * p_message, Args ... p_args )
{
	Error err{};
	err.m_errorType = static_cast<int32_t>(Sat::ErrorType::STRCOPYFAILURE);
	CStringWrapper wrap = Sat::str_format("Satellite: Error during string copy '%i'.", p_args ...);
	StrHnd strhnd = to_interface_type(std::move(wrap));
	err.m_errorMessage = strhnd;
	return err;
}

}
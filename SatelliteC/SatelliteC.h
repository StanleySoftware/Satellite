#pragma once

#include <cstdint>

#ifdef SATELLITEC_EXPORTS
#define SATELLITEC_API __declspec(dllexport)
#else
#define SATELLITEC_API __declspec(dllimport)
#endif

extern "C"
{

namespace Sat
{

class ISatellite;

}

namespace SatC
{

using StrHnd = uint32_t;
using ISatellite = Sat::ISatellite;

struct SATELLITEC_API Error
{
	StrHnd m_errorMessage;
	int32_t m_errorType;
};

struct SATELLITEC_API WorkspaceInfo
{
	StrHnd m_workspaceRoot;
	bool m_isWorkspace;
};

/// @brief Initialise the library - must be called before anything else
SATELLITEC_API bool sat_init();
/// @brief Shutdown the library
SATELLITEC_API bool sat_shutdown();

/// @brief Create a satellite object.
/// @return A pointer to the object.
SATELLITEC_API Error sat_create(ISatellite** p_out_satellite);

/// @brief Load the satellite object before doing anything with it.
/// @param p_satellite - Ptr to the target satellite object.
SATELLITEC_API void sat_load(ISatellite* p_satellite);

/// @brief Shut down the satellite object as soon as you are done with it.
SATELLITEC_API void sat_unload(ISatellite* p_satellite);

/// @brief Retrieves Workspace information for the Workspace which contains the path 'p_targetPath'.
/// @param p_satellite - Ptr to the target satellite object.
/// @param p_targetPath - The target path to inspect.
/// @param p_out_workspaceInfo - The output SatelliteWorkspaceInfo object.
/// @return An encountered error or a succesful state.
SATELLITEC_API Error sat_workspace_info(ISatellite* p_satellite, char const* p_targetPath, WorkspaceInfo* p_out_workspaceInfo);

/// @brief Invokes a relay command from the target path 'p_originPath' with the query 'p_query'.
/// Sets output parameter 'p_out_string' with the lookup response if successful.
/// Queries are colon separated keys, e.g. key1:key2:key3.
/// A colon represents a lookup of another satellite.json file, using the result
/// of the preceding key to identify which json file to load next. If the preceding
/// key does not produce a path to a satellite.json file, then the query is ill-formed.
/// The last key in the sequence does not need to resolve to a satellite.json file.
/// @param p_satellite - Ptr to the target satellite object.
/// @param p_originPath - Lookup always begins from the root of the Workspace, but this path is used to find it
/// @param p_query - The query for relay to resolve against.
/// @param p_out_string - The resulting string looked up from the query
/// @return An encountered error or a succesful state.
SATELLITEC_API Error sat_relay(ISatellite* p_satellite, char const * p_originPath, char const* p_query, StrHnd* p_out_string);

/// @brief Release a satellite object.
/// @param p_satellite - Ptr to Ptr of the target satellite object.
SATELLITEC_API void sat_release(ISatellite** p_satellite);

/// @brief Get the length of a string associated with a string handle
/// @param p_strhnd 
SATELLITEC_API uint32_t sat_length_from_strhnd(StrHnd p_strhnd);

/// @brief Copy a string associated with a string handle into a char array
/// that has already had enough space allocated by the user. To get the number
/// of chars needed for allocation, the user should call sat_length_from_strhnd().
/// @param p_strhnd - The string handle to target
/// @param p_size - The number of characters to copy
/// @param p_out_string - The char array where the string is copied to.
/// @return Any errors encountered, otherwise returns a succesful error.
SATELLITEC_API Error sat_copy_from_strhnd(StrHnd p_strhnd, uint32_t p_size, char * p_out_string);

/// @brief Clears the string table that is used to
/// store strings associated with string handles.
/// @return True if the string table was already active
/// and was shut down, otherwise false.
SATELLITEC_API bool sat_clear_string_table();

}

}
#pragma once

#ifdef SATELLITEC_EXPORTS
#define SATELLITELIB_API __declspec(dllexport)
#else
#define SATELLITELIB_API __declspec(dllimport)
#endif

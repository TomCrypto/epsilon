#pragma once

#include <string>

/** @file version.hpp
  * @brief Version information.
  *
  * This file just contains program version information.
**/

/** @brief Returns the program's version.
  * @note The version is returned in the format "vX.YY" where X is the major
  *       version number and Y is the minor version number.
**/
std::string GetRendererVersion();

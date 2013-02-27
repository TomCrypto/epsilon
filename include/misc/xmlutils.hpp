#pragma once

#include <math/vector.hpp>

#include <misc/pugixml.hpp>

/** @file xmlutils.hpp
  * @brief XML manipulation utilities
  *
  * This file contains functions which help in parsing XML, such as helper
  * functions to parse a vector out of a node, and so on.
**/

/** @brief Parses a vector out of a node with (x, y, z) attributes.
  * @param node The XML node to parse from.
  * @return The corresponding vector.
  * @note If attributes are missing, does not fail but defaults to NaN.
**/ 
Vector parseVector(pugi::xml_node node);

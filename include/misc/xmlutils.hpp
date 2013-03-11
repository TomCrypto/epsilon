#pragma once

#include <math/vector.hpp>

#include <misc/pugixml.hpp>

#include <sstream>

/** @file xmlutils.hpp
  * @brief XML manipulation utilities.
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

/** @brief Splits a string given an arbitrary delimiter.
  * @param s The string to split.
  * @param del The delimiter character.
  * @param tokens A vector to put the tokens in.
**/
void split(const std::string &s, char del, std::vector<std::string> &tokens);

/** @brief Splits a stringgiven an arbitrary delimiter.
  * @param s The string to split.
  * @param del The delimiter character.
  * @returns A vector containing the tokens.
**/
std::vector<std::string> split(const std::string &s, char del);

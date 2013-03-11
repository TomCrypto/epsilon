#include <misc/xmlutils.hpp>

#include <cmath>

Vector parseVector(pugi::xml_node node)
{
    return Vector(node.attribute("x").as_float(NAN),
                  node.attribute("y").as_float(NAN),
                  node.attribute("z").as_float(NAN));
}

void split(const std::string &s, char del, std::vector<std::string> &tokens)
{
    std::stringstream ss(s);
    std::string item;

    while(std::getline(ss, item, del)) tokens.push_back(item);
}

std::vector<std::string> split(const std::string &s, char del)
{
    std::vector<std::string> tokens;
    split(s, del, tokens);
    return tokens;
}

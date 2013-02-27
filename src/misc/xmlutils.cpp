#include <misc/xmlutils.hpp>

#include <cmath>

Vector parseVector(pugi::xml_node node)
{
    return Vector(node.attribute("x").as_float(NAN),
                  node.attribute("y").as_float(NAN),
                  node.attribute("z").as_float(NAN));
}

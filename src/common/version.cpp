#include <common/version.hpp>

#include <sstream>
#include <iomanip>

const size_t MAJOR = 0;  /* Major version number. */
const size_t MINOR = 13; /* Minor version number. */

std::string GetRendererVersion()
{
    std::stringstream stream;

    stream << "v" << std::setw(1) << std::setfill('0') << MAJOR;
    stream << "." << std::setw(2) << std::setfill('0') << MINOR;

    return stream.str();
}

#include <error/error.hpp>

std::string Error(std::string msg, int code)
{
    return msg + " [" + std::to_string(code) + "]";
}

#ifndef UTIL_STRING_HELPERS_H_
#define UTIL_STRING_HELPERS_H_

#include <string>

namespace util {

static const char *ws = " \t\n\r\f\v";

std::string &rtrim(std::string &s, const char *t = ws);

std::string &ltrim(std::string &s, const char *t = ws);

std::string &trim(std::string &s, const char *t = ws);

}  // namespace util

#endif  // UTIL_STRING_HELPERS_H_

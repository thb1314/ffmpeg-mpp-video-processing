#ifndef UTILS_HPP
#define UTILS_HPP

extern "C"
{
#include <libavutil/time.h>
}

namespace Utils
{

int64_t get_curtime();
int core_count();
}

#endif // UTILS_HPP

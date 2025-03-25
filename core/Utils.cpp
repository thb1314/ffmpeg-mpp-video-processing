#include "Utils.h"
#if !defined (_WIN32) && !defined (_WIN64)
#define LINUX
#include <cstdio>
#include <cstdlib>
#include <cstring>
#else
#define WINDOWS
#include <windows.h>
#endif

namespace Utils
{

int64_t get_curtime()
{
    return av_gettime();
}




int core_count()
{
    int core_count = 0; // 至少一个
#if defined (LINUX)
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (fp == NULL) {
        return 1;
    }

    char line[256];
    
    while (fgets(line, sizeof(line), fp)) {
        // 每个逻辑处理器会有一行 "processor"
        if (strncmp(line, "processor", 9) == 0) {
            core_count++;
        }
    }

    fclose(fp);
#elif defined (WINDOWS)
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    core_count = (int)si.dwNumberOfProcessors;
#endif
    return core_count;
}



}

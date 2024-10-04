#ifndef DEBUG_MODE
#define DEBUG_MODE true
#endif

#define printd(fmt, ...) \
    do { if (DEBUG_MODE) printf(fmt, ##__VA_ARGS__); } while (0)

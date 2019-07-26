
// #ifdef DEBUG
#define DEBUG_TEST 1
// #else
// #define DEBUG_TEST 0
// #endif

#define DEBUG_PRINT(fmt, args...)                                \
    do                                                           \
    {                                                            \
        if (DEBUG_TEST)                                          \
            fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, __FILE__, \
                    __LINE__, __FUNCTION__, ##args);             \
    } while (0)


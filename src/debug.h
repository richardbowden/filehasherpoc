
// #ifdef DEBUG
#define DEBUG_TEST 0
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


#define DEBUG_HASH(hash)\
({\
        if (DEBUG_TEST)                                                                 \
            DEBUG_PRINT("%08X-%08X-%08X-%08X\n", hash[3], hash[2], hash[1], hash[0]);     \
                        }) \

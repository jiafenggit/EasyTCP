#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H

#define TEST_DEFAULT_DATA_SIZ       1024

typedef struct
{
    uint64_t countRead;
    uint64_t countWrite;
    uint64_t bytesRead;
    uint64_t bytesWrite;
} Count;

#endif

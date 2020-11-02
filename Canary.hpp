#ifndef CANARY_HPP
#define CANARY_HPP

#define FREE_BUF_WITH_CANARY_BORDER(ptr) free((char *) (ptr) - CanarySize)

typedef unsigned long long canary_t;

enum Canary {
    CanarySize = sizeof(canary_t),
    CanaryValue = 0xDEADBEDull
};

canary_t leftBufCanary(const void *buf);
canary_t rightBufCanary(const void *buf, size_t bufSize);

void *callocBufWithBorderCanaries(size_t bufSize, size_t bufElemSize);

void *reallocBufWithBorderCanaries(void *buf, size_t newBufSize);

#endif /* CANARY_HPP */

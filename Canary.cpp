#include "Canary.hpp"

#include <cassert>
#include <cstdlib>

canary_t leftBufCanary(const void *buf)
{
    assert(buf != nullptr);

    return *(canary_t *)(((char *) buf) - CanarySize);
}

canary_t rightBufCanary(const void *buf, size_t bufSize)
{
    assert(buf != nullptr);

    return *(canary_t *)(((char *) buf) + bufSize);
}

void *callocBufWithBorderCanaries(size_t bufSize, size_t bufElemSize)
{
    auto buffer = (char *) std::calloc(1, CanarySize + bufSize * bufElemSize + CanarySize);
    *((canary_t *) buffer) = *((canary_t *) (buffer + CanarySize + bufSize * bufElemSize)) = CanaryValue;
    return buffer + CanarySize;
}

void *reallocBufWithBorderCanaries(void *buf, size_t newBufSize)
{
    buf = realloc((char *) buf - CanarySize, CanarySize + newBufSize + CanarySize);
    *((canary_t *) buf) = *((canary_t *) ((char *) buf + CanarySize + newBufSize)) = CanaryValue;
    return (char *) buf + CanarySize;
}

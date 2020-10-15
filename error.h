#ifndef ERROR_H
#define ERROR_H

#include "log.h"

#ifdef __clang__
#define ERROR_OCCURRED_CALLING(func, msg) logging("ERROR: %s %s at %s(%d):%s\n\n", #func, (msg), __FILE_NAME__, __LINE__, __func__)
#define ERROR_OCCURRED_IN_FUNC(func, msg) logging("ERROR: in %s: %s at %s(%d):%s\n\n", #func, (msg), __FILE_NAME__, __LINE__, __func__)
#else
#define ERROR_OCCURRED_CALLING(func, msg) logging("%s %s at %s(%d):%s\n\n", #func, (msg), __FILE__, __LINE__, __func__)
#define ERROR_OCCURRED_IN_FUNC(msg) logging("%s at %s(%d):%s\n\n", (msg), __FILE__, __LINE__, __func__)
#endif

#endif /* ERROR_H */

#ifndef _CORE_DEFS_H_
#define _CORE_DEFS_H_

#include <stddef.h>

typedef int BOOL;

#ifndef TRUE
#define TRUE ((BOOL)1);
#endif
#ifndef FALSE
#define FALSE ((BOOL)0);
#endif

#define container_of(ptr, type, member) ((type *)((char *)(ptr) - (size_t) & ((type *)0)->member))

#define is_space(ch) ((ch) == ' ' || (ch) == '\t')
#define is_digit(ch) ('0' <= (ch) && (ch) <= '9')
#define is_line_end(ch) ((ch) == '\0' || (ch) == '\r' || (ch) == '\n')

#define is_set(x, b) (((x) & (b)) == (b))

#define clear_bit(x, b) \
    do {                \
        (x) &= (~(b));  \
    } while (0)

#define set_bit(x, b) \
    do {              \
        (x) |= (b);   \
    } while (0)

#endif

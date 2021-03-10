#ifndef _CORE_DEFS_H_
#define _CORE_DEFS_H_

#include <stddef.h>

typedef int BOOL;
typedef long long money_t;

#ifndef TRUE
#define TRUE ((BOOL)1);
#endif
#ifndef FALSE
#define FALSE ((BOOL)0);
#endif

#define container_of(ptr, type, member) ((type *)((char *)(ptr) - (size_t) & ((type *)0)->member))

#define is_space(ch)    ((ch) == ' ' || (ch) == '\t')
#define is_line_end(ch) ((ch) == '\0' || (ch) == '\r' || (ch) == '\n')

#define is_digit(ch)    ('0' <= (ch) && (ch) <= '9')
#define is_hex(ch)      ('A' <= (ch) && (ch) <= 'F' || 'a' <= (ch) && (ch) <= 'f')
#define digit_value(ch) ((ch)&0x0F)
#define hex_value(ch)   (((ch)&0x0F) + 9)

#define skip_space(p, buf) for ((p) = (buf); is_space(*(p)) && !is_line_end(*(p)); (p)++)

#define is_set(x, b) (((x) & (b)) == (b))

#define clear_bit(x, b)                                                                                                \
    do {                                                                                                               \
        (x) &= (~(b));                                                                                                 \
    } while (0)

#define set_bit(x, b)                                                                                                  \
    do {                                                                                                               \
        (x) |= (b);                                                                                                    \
    } while (0)

#endif

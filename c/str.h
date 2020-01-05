#ifndef _STR_H_
#define _STR_H_

#include "core_defs.h"

struct string {
    char *str;
    size_t len;
    BOOL own;
};

#ifdef __cplusplus
extern "C" {
#endif

void string_init(struct string *str);
void string_release(struct string *str);

struct string *string_copy(struct string *dst, const struct string *src);
struct string *cstr_copy(struct string *dst, const char *src, size_t len);
struct string *string_mock(struct string *dst, char *buf, size_t len);
size_t string_mock_slice(struct string *dst, const char *buf, char end);

int string_compare(const struct string *str1, const struct string *str2);

BOOL string_is_empty(const struct string *str);

#ifdef __cplusplus
}
#endif

#endif

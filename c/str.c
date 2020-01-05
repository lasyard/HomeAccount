#include <stdlib.h>
#include <string.h>

#include "str.h"

void string_init(struct string *str)
{
    str->str = NULL;
    str->len = 0;
    str->own = FALSE;
}

void string_release(struct string *str)
{
    if (str->own) free(str->str);
}

static struct string *_cstr_put(struct string *dst, char *src, size_t len)
{
    string_release(dst);
    dst->str = src;
    dst->len = len;
    dst->own = TRUE;
    return dst;
}

struct string *string_copy(struct string *dst, const struct string *src)
{
    return cstr_copy(dst, src->str, src->len);
}

struct string *cstr_copy(struct string *dst, const char *src, size_t len)
{
    char *s;
    if ((s = (char *)malloc(len + 1)) == NULL) return NULL;
    memcpy(s, src, len);
    s[len] = '\0';
    return _cstr_put(dst, s, len);
}

struct string *string_mock(struct string *dst, char *buf, size_t len)
{
    dst->str = buf;
    dst->len = len;
    dst->own = FALSE;
    return dst;
}

size_t string_mock_slice(struct string *dst, const char *buf, char end)
{
    const char *p;
    int len;
    dst->own = FALSE;
    for (p = buf; is_space(*p) && !is_line_end(*p); p++)
        ;
    dst->str = (char *)p;
    for (len = 0; *p != end && !is_line_end(*p); p++) {
        if (is_space(*p)) {
            if (len == 0) len = p - dst->str;
        } else {
            len = 0;
        }
    }
    if (len == 0) len = p - dst->str;
    dst->len = len;
    return p - buf;
}

int string_compare(const struct string *str1, const struct string *str2)
{
    int result;
    if (str1->len < str2->len) {
        result = memcmp(str1->str, str2->str, str1->len);
        if (result == 0) return -1;
    } else {
        result = memcmp(str1->str, str2->str, str2->len);
        if (result == 0 && str1->len > str2->len) return 1;
    }
    return result;
}

BOOL string_is_empty(const struct string *str)
{
    return (str->str == NULL) || str->len == 0;
}

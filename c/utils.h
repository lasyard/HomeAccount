#ifndef _UTILS_H_
#define _UTILS_H_

#include "core_defs.h"

#define YEAR_LEN 4
#define MONTH_LEN 2
#define DAY_LEN 2
#define DATE_LEN (YEAR_LEN + 1 + MONTH_LEN + 1 + DAY_LEN)
#define PATH_LEN (YEAR_LEN + 1 + MONTH_LEN + 4)

#define MONEY_LEN (12 + 1)

#define PRECISION 2
#define MUL_NUM 100

#ifdef __cplusplus
extern "C" {
#endif

int str_to_int(const char *str, size_t len);
size_t int_to_str_len(char *buf, long long num, size_t len);
size_t int_to_str(char *buf, long long num);

size_t money_to_str(char *buf, money_t x);
money_t str_to_money(const char *buf);

size_t parse_str_to_money(const char *buf, money_t *money);

int last_day_of_month(int year, int month);

BOOL str_to_ym(const char *str, int *year, int *month, char sep);
BOOL str_to_ymd(const char *str, int *year, int *month, int *day, char sep);
size_t ym_to_str(char *str, int year, int month, char sep);
size_t ymd_to_str(char *str, int year, int month, int day, char sep);

#ifdef __cplusplus
}
#endif

#endif

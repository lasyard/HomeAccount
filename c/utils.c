#include "core_defs.h"

#include "utils.h"

int str_to_int(const char *str, size_t len)
{
    int num = 0;
    const char *p;
    int i = 0;
    for (p = str; i < len; p++, i++) {
        if (*p < '0' || *p > '9') return 0;
        num = num * 10 + (*p & 0x0F);
    }
    return num;
}

size_t int_to_str_len(char *buf, int num, size_t len)
{
    char *p;
    for (p = buf + len - 1; p >= buf; p--) {
        *p = num % 10 | ('0' & 0xF0);
        num /= 10;
    }
    return len;
}

size_t int_to_str(char *buf, int num)
{
    int len = 1;
    for (int power = 1; power <= num / 10; power *= 10) {
        len++;
    }
    return int_to_str_len(buf, num, len);
}

size_t money_to_str(char *buf, long x)
{
    char *p = buf;
    if (x < 0) {
        *p++ = '-';
        x = -x;
    }
    p += int_to_str(p, x / MUL_NUM);
    x %= MUL_NUM;
    *p++ = '.';
    p += int_to_str_len(p, x, PRECISION);
    *p = '\0';
    return p - buf;
}

long str_to_money(const char *buf)
{
    long money;
    parse_str_to_money(buf, &money);
    return money;
}

size_t parse_str_to_money(const char *buf, long *money)
{
    int neg = 0;
    int decimal = 0;
    int money_parsed = 0;
    int m = 0;
    const char *p;
    for (p = buf; is_space(*p) && !is_line_end(*p); p++)
        ;
    if (*p == '-') {
        neg = 1;
        p++;
    } else if (*p == '+') {
        p++;
    }
    for (; is_digit(*p); p++) {
        m = m * 10 + (long)(*p & 0x0F);
        money_parsed = 1;
    }
    m *= MUL_NUM;
    if (*p == '.') {
        decimal = 1;
        for (p++; is_digit(*p); p++) {
            int i;
            long tmp;
            if (decimal > PRECISION) return -1;
            tmp = (long)(*p & 0x0F);
            for (i = decimal; i < PRECISION; i++) tmp *= 10;
            m += tmp;
            decimal++;
            money_parsed = 1;
        }
    }
    if (!money_parsed) return 0;
    if (neg) m = -m;
    *money = m;
    return p - buf;
}

int last_day_of_month(int year, int month)
{
    if (month == 2) {
        if (year % 4 == 0 && year % 400 != 0) {
            return 29;
        } else {
            return 28;
        }
    } else if (month == 4 || month == 6 || month == 9 || month == 11) {
        return 30;
    } else {
        return 31;
    }
}

BOOL str_to_ym(const char *str, int *year, int *month, char sep)
{
    const char *p = str;
    if ((*year = str_to_int(p, YEAR_LEN)) == 0) return FALSE;
    p += YEAR_LEN;
    if (sep != '\0') {
        if (*p != sep) return FALSE;
        p++;
    }
    if ((*month = str_to_int(p, MONTH_LEN)) < 1 || *month > 12) return FALSE;
    return TRUE;
}

BOOL str_to_ymd(const char *str, int *year, int *month, int *day, char sep)
{
    const char *p = str;
    if (!str_to_ym(str, year, month, sep)) return FALSE;
    p += YEAR_LEN + MONEY_LEN;
    if (sep != '\0') {
        if (*p != sep) return FALSE;
        p++;
        p++;
    }
    if ((*day = str_to_int(p, DAY_LEN)) < 1 || *day > last_day_of_month(*year, *month)) {
        return FALSE;
    }
    return TRUE;
}

size_t ym_to_str(char *str, int year, int month, char sep)
{
    char *p = str;
    p += int_to_str_len(p, year, YEAR_LEN);
    if (sep != '\0') *p++ = sep;
    p += int_to_str_len(p, month, MONTH_LEN);
    return p - str;
}

size_t ymd_to_str(char *str, int year, int month, int day, char sep)
{
    char *p = str;
    p += ym_to_str(p, year, month, sep);
    if (sep != '\0') *p++ = sep;
    p += int_to_str_len(p, day, DAY_LEN);
    *p = '\0';
    return p - str;
}

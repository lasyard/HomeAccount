#include "utils.h"

int str_to_int(const char *str, size_t len)
{
    int num = 0;
    const char *p;
    int i = 0;
    for (p = str; i < len; p++, i++) {
        if (!is_digit(*p)) {
            return 0;
        }
        num = num * 10 + digit_value(*p);
    }
    return num;
}

size_t int_to_str_len(char *buf, long long num, size_t len)
{
    char *p;
    for (p = buf + len - 1; p >= buf; p--) {
        *p = num % 10 | ('0' & 0xF0);
        num /= 10;
    }
    return len;
}

size_t int_to_str(char *buf, long long num)
{
    int len = 1;
    for (long long power = 1; power <= num / 10; power *= 10) {
        len++;
    }
    return int_to_str_len(buf, num, len);
}

size_t money_to_str(char *buf, money_t x)
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

size_t time_to_str(char *buf, time_t t)
{
    t /= TIME_SCALE;
    for (char *p = buf + 7; p >= buf; p--, t >>= 4) {
        time_t v = t & 0x0000000F;
        if (v < 10) {
            *p = v + '0';
        } else {
            *p = v - 10 + 'A';
        }
    }
    buf[8] = '\0';
    return 8;
}

money_t str_to_money(const char *buf)
{
    money_t money = 0;
    parse_str_to_money(buf, &money);
    return money;
}

size_t parse_time(const char *buf, time_t *time)
{
    time_t t = 0;
    const char *p;
    skip_space(p, buf);
    if (is_line_end(*p)) {
        return 0;
    }
    for (;; p++) {
        time_t v;
        if (is_digit(*p)) {
            v = digit_value(*p);
        } else if (is_hex(*p)) {
            v = hex_value(*p);
        } else {
            break;
        }
        t <<= 4;
        t |= v;
    }
    *time = t * TIME_SCALE;
    return p - buf;
}

size_t parse_str_to_money(const char *buf, money_t *money)
{
    BOOL neg = FALSE;
    int decimal = 0;
    BOOL money_parsed = FALSE;
    money_t m = 0;
    const char *p;
    skip_space(p, buf);
    if (is_line_end(*p)) {
        return 0;
    }
    if (*p == '-') {
        neg = TRUE;
        p++;
    } else if (*p == '+') {
        p++;
    }
    for (; is_digit(*p); p++) {
        m = m * 10 + (money_t)digit_value(*p);
        money_parsed = TRUE;
    }
    m *= MUL_NUM;
    if (*p == '.') {
        decimal = 1;
        for (p++; is_digit(*p); p++) {
            int i;
            money_t tmp;
            if (decimal > PRECISION) {
                return -1;
            }
            tmp = (money_t)digit_value(*p);
            for (i = decimal; i < PRECISION; i++)
                tmp *= 10;
            m += tmp;
            decimal++;
            money_parsed = TRUE;
        }
    }
    if (!money_parsed) {
        return 0;
    }
    if (neg) {
        m = -m;
    }
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
    if ((*year = str_to_int(p, YEAR_LEN)) == 0) {
        return FALSE;
    }
    p += YEAR_LEN;
    if (sep != '\0') {
        if (*p != sep) {
            return FALSE;
        }
        p++;
    }
    if ((*month = str_to_int(p, MONTH_LEN)) < 1 || *month > 12) {
        return FALSE;
    }
    return TRUE;
}

BOOL str_to_ymd(const char *str, int *year, int *month, int *day, char sep)
{
    const char *p = str;
    if (!str_to_ym(str, year, month, sep)) {
        return FALSE;
    }
    p += YEAR_LEN + MONTH_LEN;
    if (sep != '\0') {
        p++;
        if (*p != sep) {
            return FALSE;
        }
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
    if (sep != '\0') {
        *p++ = sep;
    }
    p += int_to_str_len(p, month, MONTH_LEN);
    return p - str;
}

size_t ymd_to_str(char *str, int year, int month, int day, char sep)
{
    char *p = str;
    p += ym_to_str(p, year, month, sep);
    if (sep != '\0') {
        *p++ = sep;
    }
    p += int_to_str_len(p, day, DAY_LEN);
    *p = '\0';
    return p - str;
}

size_t timestamp_to_str(char *buf, time_t t)
{
    char *p = buf;
    if (t > 0) {
        struct tm tm;
        localtime_r(&t, &tm);
        p += ymd_to_str(p, tm.tm_year + 1900, tm.tm_mon, tm.tm_mday, '.');
        *p++ = ' ';
        p += int_to_str_len(p, tm.tm_hour, 2);
        *p++ = ':';
        p += int_to_str_len(p, tm.tm_min, 2);
    }
    *p = '\0';
    return p - buf;
}

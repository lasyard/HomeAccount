#include "DataFileRW.h"
#include "except/DataFileError.h"

void DataFileRW::parseLine(const char *line)
{
    const char *p = line;
    if (*p == '#') {
        struct string title;
        string_mock_slice(&title, p + 1, '\0');
        if (add_page(m_dt, &title) == NULL) {
            throw std::bad_alloc();
        }
        return;
    }
    int len;
    money_t money = 0;
    if ((len = parse_str_to_money(p, &money)) <= 0) {
        throw DataFileError();
    }
    p += len;
    struct string desc;
    string_init(&desc);
    if (is_space(*p)) {
        p++;
        p += string_mock_slice(&desc, p, ':');
    }
    struct string comment;
    string_init(&comment);
    if (*p == ':') {
        p++;
        p += string_mock_slice(&comment, p, '\0');
    }
    if (ulist_is_empty(&m_dt->pages)) {
        m_dt->initial -= money;
    } else {
        if (add_item(get_page(m_dt->pages.last), money, &desc, &comment) == NULL) {
            throw std::bad_alloc();
        }
    }
}

void DataFileRW::writeItem(std::ostream &os, const struct item *it) const
{
    char buf[MONEY_LEN + 1];
    if (is_dummy_item(it)) return;
    money_to_str(buf, it->money);
    os << buf;
    if (it->desc.len > 0) {
        os << '\t' << it->desc.str;
    }
    if (it->comment.len > 0) {
        os << ':' << it->comment.str;
    }
    os << std::endl;
}

void DataFileRW::writePage(std::ostream &os, const struct page *pg) const
{
    struct ulist_item *p;
    os << '#' << pg->title.str << std::endl;
    if (is_empty_page(pg)) return;
    for (p = pg->items.first; p != NULL; p = p->next) writeItem(os, get_item(p));
}

void DataFileRW::writeData(std::ostream &os, const struct data *dt) const
{
    struct ulist_item *p;
    writeInitial(os, dt->initial);
    if (is_empty_data(m_dt)) return;
    for (p = m_dt->pages.first; p != NULL; p = p->next) writePage(os, get_page(p));
}

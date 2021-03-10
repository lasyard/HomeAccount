#include "DataFileRW.h"
#include "except/DataFileError.h"

void DataFileRW::parseLine(const char *line)
{
    err_code err = parse_data(m_dt, line);
    switch (err) {
    case ERR_BAD_ALLOC:
        throw std::bad_alloc();
    case ERR_DATA_FILE:
        throw DataFileError();
    default:
        break;
    }
}

void DataFileRW::writeItem(std::ostream &os, const struct item *it) const
{
    char buf[MONEY_LEN + 1];
    if (is_dummy_item(it)) {
        return;
    }
    if (it->time > 0) {
        // money is longer than time.
        time_to_str(buf, it->time);
        os << '@' << buf << ' ';
    }
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
    if (is_empty_page(pg)) {
        return;
    }
    for (p = pg->items.first; p != NULL; p = p->next) {
        writeItem(os, get_item(p));
    }
}

void DataFileRW::writeData(std::ostream &os) const
{
    struct ulist_item *p;
    writeInitial(os, m_dt->initial);
    if (is_empty_data(m_dt)) {
        return;
    }
    for (p = m_dt->pages.first; p != NULL; p = p->next) {
        writePage(os, get_page(p));
    }
}

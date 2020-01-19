#include "FileRW.h"
#include "except/FileError.h"

void FileRW::readStream(std::istream &is)
{
    int lineNo = 0;
    while (is.getline(m_buf, LINE_LEN)) {
        lineNo++;
        try {
            if (is_line_end(m_buf[0])) continue;
            parseLine(m_buf);
        } catch (FileError &e) {
            e.setFileName(m_fileName);
            e.setLineNo(lineNo);
            throw e;
        }
    }
}

#include "FileRW.h"
#include "except/FileError.h"

void FileRW::readStream(std::istream &is)
{
    int lineNo(0);
    char buf[LINE_LEN];
    while (is.getline(buf, LINE_LEN)) {
        lineNo++;
        try {
            if (is_line_end(buf[0])) continue;
            parseLine(buf);
        } catch (FileError &e) {
            e.setFileName(m_fileName);
            e.setLineNo(lineNo);
            throw e;
        }
    }
}

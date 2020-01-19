#include "FileRW.h"
#include "except/FileError.h"

const char *const FileRW::FILE_SEPARATOR = "~~~";

void FileRW::readStream(std::istream &is)
{
    int lineNo = 0;
    while (is.getline(m_buf, LINE_LEN)) {
        lineNo++;
        try {
            if (is_line_end(m_buf[0])) continue;
            if (strncmp(m_buf, FILE_SEPARATOR, 3) == 0) return;
            parseLine(m_buf);
        } catch (FileError &e) {
            e.setFileName(m_fileName);
            e.setLineNo(lineNo);
            throw;
        }
    }
}

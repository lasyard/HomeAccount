#ifndef _FILE_R_W_H_
#define _FILE_R_W_H_

#include <fstream>

#include "HaFile.h"
#include "except/InternalError.h"

class FileRW
{
public:
    static const int LINE_LEN = 256;
    static const int TYPE_HEADER_LEN = 4;
    static const char *const FILE_SEPARATOR;

    FileRW(const char *fileName, HaFile *file = nullptr) : m_fileName(fileName), m_modified(false), m_file(file)
    {
    }

    virtual ~FileRW()
    {
    }

    void save()
    {
        if (m_file == nullptr) return;
        m_file->save(this);
        m_modified = false;
    }

    virtual void afterSave() const = 0;

    void saveAs(const std::string &path) const
    {
        std::ofstream file(path);
        saveAs(file);
    }

    void saveAs(std::ostream &os) const
    {
        os << typeHeader() << ": exported by " << HaFile::idString() << std::endl;
        writeStream(os);
    }

    void readStream(std::istream &is);

    void writeStream(std::ostream &os) const
    {
        writeData(os);
    }

    void setModified()
    {
        m_modified = true;
    }

    bool modified() const
    {
        return m_modified;
    }

    const char *fileName() const
    {
        return m_fileName.c_str();
    }

    void setHaFile(HaFile *haFile)
    {
        m_file = haFile;
    }

    virtual void clearData() = 0;

    virtual bool import(const char *header, std::istream &is)
    {
        if (!parseHeader(header)) return false;
        clearData();
        readStream(is);
        return true;
    }

    virtual const HaFile::FileType type()
    {
        throw InternalError("Should not run to here: %s : %d", __FILE__, __LINE__);
    }

    void writeSeparator(std::ostream &os)
    {
        os << FILE_SEPARATOR << std::endl;
    }

protected:
    std::string m_fileName;
    bool m_modified;
    HaFile *m_file;
    char m_buf[LINE_LEN];

    virtual const char *typeHeader() const = 0;
    virtual void parseLine(const char *line) = 0;
    virtual void writeData(std::ostream &os) const = 0;

    virtual bool parseHeader(const char *line)
    {
        if (strncmp(line, typeHeader(), TYPE_HEADER_LEN) == 0) return true;
        return false;
    }
};

#endif

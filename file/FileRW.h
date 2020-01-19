#ifndef _FILE_R_W_H_
#define _FILE_R_W_H_

#include <fstream>

#include "HaFile.h"
#include "except/InternalError.h"

class FileRW
{
public:
    static const int LINE_LEN = 256;
    static const int HEADER_LEN = 4;

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
        file << header() << ": exported by " << HaFile::idString() << std::endl;
        writeStream(file);
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

    virtual bool import(const char *header, std::ifstream &ifs)
    {
        if (!parseHeader(header)) return false;
        clearData();
        readStream(ifs);
        return true;
    }

    virtual const HaFile::FileType type()
    {
        throw InternalError("Should not run to here: %s : %d", __FILE__, __LINE__);
    }

protected:
    std::string m_fileName;
    bool m_modified;
    HaFile *m_file;
    char m_buf[LINE_LEN];

    virtual const char *header() const = 0;
    virtual void parseLine(const char *line) = 0;
    virtual void writeData(std::ostream &os) const = 0;

    virtual bool parseHeader(const char *line)
    {
        if (strncmp(line, header(), HEADER_LEN) == 0) return true;
        return false;
    }
};

#endif

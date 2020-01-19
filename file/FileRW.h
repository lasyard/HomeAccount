#ifndef _FILE_R_W_H_
#define _FILE_R_W_H_

#include <fstream>

#include "HaFile.h"

class FileRW
{
public:
    static const int LINE_LEN = 256;

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
        writeStream(file);
    }

    void readStream(std::istream &is);

    void writeStream(std::ostream &os) const
    {
        os << header() << ": exported by " << HaFile::idString() << std::endl;
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

    void import(const std::string &path)
    {
        clearData();
        std::ifstream ifs(path);
        readStream(ifs);
    }

protected:
    std::string m_fileName;
    bool m_modified;
    HaFile *m_file;

    virtual void parseLine(const char *line) = 0;
    virtual wxString header() const = 0;
    virtual void writeData(std::ostream &os) const = 0;
};

#endif

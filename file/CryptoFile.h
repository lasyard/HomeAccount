#ifndef _CRYPTO_FILE_H_
#define _CRYPTO_FILE_H_

#include <fstream>
#include <map>
#include <sstream>

#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/strconv.h>

#include "../c/str.h"

class CryptoFile
{
public:
    static const int KEY_LEN = 16;
    static const int IV_LEN = 32;

    CryptoFile(const char *fileName) : m_fileName(fileName)
    {
    }

    CryptoFile(const char *fileName, const std::string &iv) : m_fileName(fileName), m_catalog()
    {
        std::string output;
        digest("", output);
        memcpy(m_key, output.c_str(), output.length());
        digest(iv, output);
        memcpy(m_iv, output.c_str(), output.length());
        srand(time(NULL));
    }

    virtual ~CryptoFile()
    {
        delete m_mbConv;
    }

    void setKey(const std::string &key)
    {
        std::string output;
        digest(key, output);
        memcpy(m_key, output.c_str(), output.length());
    }

    void changeKey(const std::string &key)
    {
        setKey(key);
        saveCatalog();
        m_file.flush();
    }

    void copyFrom(CryptoFile *file);

    bool copyTo(const char *dirName)
    {
        wxString path, name, ext;
        wxFileName::SplitPath(m_fileName, &path, &name, &ext);
        return wxCopyFile(m_fileName, wxFileName(dirName, name + "." + ext).GetFullPath(), true);
    }

    bool exist(const std::string fileName) const
    {
        return m_catalog.find(fileName) != m_catalog.end();
    }

    void makeNewFile();

    void flush()
    {
        m_file.flush();
    }

protected:
    static const int HEADER_LEN = 2;
    static const int OFFSET_LEN = 4;
    static const int SIZE_LEN = 4;
    static const int FILE_NAME_LEN = 16;
    static const char *const GB2312_HEADER;
    static const char *const UTF8_HEADER;
    static const char *const CATALOG_NAME;

    void writeOffsetSize(std::ostream &os, off_t offset, size_t size)
    {
        os.seekp(HEADER_LEN, std::ios::beg);
        os.write((char *)&offset, OFFSET_LEN);
        os.write((char *)&size, SIZE_LEN);
    }

    struct FileInfo;

    struct CatRecord {
        char fileName[FILE_NAME_LEN];
        char offset[OFFSET_LEN];
        char size[SIZE_LEN];
        char key[KEY_LEN];
        CatRecord()
        {
        }
        CatRecord(const std::string &file, const FileInfo &info);
    };

    struct FileInfo {
        off_t offset;
        size_t size;
        unsigned char key[KEY_LEN];
        FileInfo()
        {
        }
        FileInfo(const std::string &k)
        {
            memcpy(key, k.c_str(), KEY_LEN);
        }
        FileInfo(off_t off, size_t len, const unsigned char *k) : offset(off), size(len)
        {
            memcpy(key, k, KEY_LEN);
        }
        FileInfo(const struct CatRecord &rec);
        FileInfo &operator=(const FileInfo &obj)
        {
            offset = obj.offset;
            size = obj.size;
            memcpy(key, obj.key, KEY_LEN);
            return *this;
        }
    };

    struct CompareFileInfoPtr {
        bool operator()(const FileInfo *a, const FileInfo *b)
        {
            return a->offset < b->offset;
        }
    };

    std::string m_fileName;
    std::fstream m_file;
    unsigned char m_key[KEY_LEN];
    unsigned char m_iv[IV_LEN];
    std::map<std::string, FileInfo> m_catalog;
    wxMBConv *m_mbConv;

    off_t findSlot(size_t length);
    void tryLoadFile();
    void loadCatalog();
    void saveCatalog();

    bool catalogLoaded() const
    {
        return m_catalog.find(CATALOG_NAME) != m_catalog.end();
    }

    void transEncoding(std::stringstream &in, std::ostream &out, wxMBConv *inC, wxMBConv *outC);
    void loadFile(const std::string &fileName, std::ostream &result);
    void saveFile(const std::string &fileName, std::stringstream &content);
    void rawSaveFile(const std::string fileName, std::istream &content, unsigned char key[KEY_LEN]);
    char *readSection(off_t offset, size_t length);
    void decSection(off_t offset, size_t length, std::ostream &output, unsigned char key[KEY_LEN] = NULL);
    void encStream(std::istream &input, std::string &output, unsigned char key[KEY_LEN] = NULL);
    void digest(const std::string &input, std::string &output);
    void encrypt(const std::string &input, std::string &output, unsigned char key[KEY_LEN]);
    void decrypt(const char *buf, size_t length, std::string &output, unsigned char key[KEY_LEN]);
    void compress(std::istream &input, std::string &output);
    void decompress(const std::string &input, std::ostream &output);
};

#endif

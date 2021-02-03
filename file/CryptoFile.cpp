#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <aes.h>
#include <files.h>
#include <gcm.h>
#include <md5.h>
#include <zlib.h>

#include "CryptoFile.h"
#include "except/BadPassword.h"
#include "except/FileCorrupt.h"

using namespace CryptoPP;

const char *const CryptoFile::GB2312_HEADER = "HA";
const char *const CryptoFile::UTF8_HEADER = "HB";
const char *const CryptoFile::CATALOG_NAME = "__catalog__";

off_t CryptoFile::findSlot(size_t length)
{
    std::vector<const FileInfo *> vec;
    for (auto i = m_catalog.cbegin(); i != m_catalog.cend(); ++i) {
        vec.push_back(&i->second);
    }
    CompareFileInfoPtr cp;
    std::sort(vec.begin(), vec.end(), cp);
    off_t offset = HEADER_LEN + OFFSET_LEN + SIZE_LEN;
    for (std::vector<const FileInfo *>::const_iterator i = vec.cbegin(); i != vec.cend(); ++i) {
        if ((size_t)(*i)->offset - (size_t)offset >= length) {
            break;
        }
        offset = (*i)->offset + (*i)->size;
    }
    return offset;
}

void CryptoFile::clearFile()
{
    m_catalog.clear();
    m_file.close();
    std::ofstream file(m_fileName, std::ios::binary);
    file.write(UTF8_HEADER, HEADER_LEN);
    std::stringstream stream;
    std::string output;
    encStream(stream, output);
    off_t offset = HEADER_LEN + OFFSET_LEN + SIZE_LEN;
    size_t length = output.length();
    file.seekp(offset, std::ios::beg);
    file.write(output.c_str(), length);
    writeOffsetSize(file, offset, length);
    file.close();
}

void CryptoFile::copyFrom(CryptoFile *file)
{
    clearFile();
    tryLoadFile();
    file->tryLoadFile();
    memcpy(m_key, file->m_key, KEY_LEN);
    memcpy(m_iv, file->m_iv, IV_LEN);
    for (auto i = file->m_catalog.cbegin(); i != file->m_catalog.cend(); ++i) {
        if (i->first == CATALOG_NAME) {
            continue;
        }
        std::stringstream ss;
        file->loadFile(i->first, ss);
        saveFile(i->first, ss);
    }
    saveCatalog();
}

void CryptoFile::tryLoadFile()
{
    if (!m_file.is_open()) {
        m_file.open(m_fileName, std::ios::binary | std::ios::in | std::ios::out);
    }
    if (!catalogLoaded()) {
        loadCatalog();
    }
}

void CryptoFile::loadCatalog()
{
    char head[HEADER_LEN + 1];
    m_file.seekg(0, std::ios::beg);
    m_file.read(head, HEADER_LEN);
    head[HEADER_LEN] = '\0';
    if (strcmp(head, GB2312_HEADER) == 0) {
        m_mbConv = new wxCSConv(wxFONTENCODING_GB2312);
    } else if (strcmp(head, UTF8_HEADER) == 0) {
        m_mbConv = new wxMBConvUTF8;
    } else {
        throw FileCorrupt();
    }
    off_t offset = 0;
    size_t length = 0;
    m_file.read((char *)&offset, OFFSET_LEN);
    m_file.read((char *)&length, SIZE_LEN);
    std::stringstream output;
    decSection(offset, length, output);
    m_catalog.clear();
    m_catalog[CATALOG_NAME] = FileInfo(offset, length, m_key);
    CatRecord rec;
    while (1) {
        output.read((char *)&rec, sizeof(rec));
        if ((size_t)output.gcount() < sizeof(rec)) {
            break;
        }
        m_catalog[rec.fileName] = FileInfo(rec);
    }
}

void CryptoFile::saveCatalog()
{
    std::stringstream stream;
    for (auto i = m_catalog.cbegin(); i != m_catalog.cend(); ++i) {
        if (i->first == CATALOG_NAME) {
            continue;
        }
        CatRecord cr(i->first, i->second);
        stream.write((char *)&cr, sizeof(CatRecord));
    }
    rawSaveFile(CATALOG_NAME, stream, m_key);
    writeOffsetSize(m_file, m_catalog[CATALOG_NAME].offset, m_catalog[CATALOG_NAME].size);
}

void CryptoFile::transEncoding(std::stringstream &in, std::ostream &out, wxMBConv *inC, wxMBConv *outC)
{
    std::string str = in.str();
    wxScopedCharBuffer scbuf = wxScopedCharBuffer::CreateNonOwned(str.data(), str.length());
    wxCharBuffer cbuf;
    cbuf = outC->cWC2MB(inC->cMB2WC(scbuf));
    out.write(cbuf.data(), cbuf.length());
}

void CryptoFile::loadFile(const std::string &fileName, std::ostream &result)
{
    if (exist(fileName)) {
        off_t offset = m_catalog[fileName].offset;
        size_t length = m_catalog[fileName].size;
        unsigned char *key = m_catalog[fileName].key;
        std::stringstream content;
        decSection(offset, length, content, key);
        transEncoding(content, result, m_mbConv, wxConvCurrent);
    }
}

void CryptoFile::saveFile(const std::string &fileName, std::stringstream &content)
{
    std::string key("0123456789ABCDEF");
    std::generate(key.begin(), key.end(), rand);
    std::string result;
    digest(key, result);
    std::stringstream ss;
    transEncoding(content, ss, wxConvCurrent, m_mbConv);
    rawSaveFile(fileName, ss, (unsigned char *)result.c_str());
}

void CryptoFile::rawSaveFile(const std::string fileName, std::istream &content, unsigned char key[KEY_LEN])
{
    std::string output;
    encStream(content, output, key);
    m_catalog.erase(fileName);
    off_t offset = findSlot(output.length());
    m_catalog[fileName] = FileInfo(offset, output.length(), key);
    m_file.seekp(offset, std::ios::beg);
    m_file.write(output.c_str(), output.length());
}

char *CryptoFile::readSection(off_t offset, size_t length)
{
    char *buf = new char[length];
    m_file.seekg(offset, std::ios::beg);
    m_file.read(buf, length);
    return buf;
}

void CryptoFile::decSection(off_t offset, size_t length, std::ostream &output, unsigned char key[KEY_LEN])
{
    if (key == nullptr) {
        key = m_key;
    }
    char *buf = readSection(offset, length);
    std::string tmp;
    try {
        decrypt(buf, length, tmp, key);
    } catch (CryptoPP::HashVerificationFilter::HashVerificationFailed &) {
        delete buf;
        throw BadPassword();
    }
    decompress(tmp, output);
    delete buf;
}

void CryptoFile::encStream(std::istream &input, std::string &output, unsigned char key[KEY_LEN])
{
    if (key == nullptr) {
        key = m_key;
    }
    std::string tmp;
    compress(input, tmp);
    encrypt(tmp, output, key);
}

void CryptoFile::digest(const std::string &input, std::string &output)
{
    Weak1::MD5 encoder;
    StringSource(input, true, new HashFilter(encoder, new StringSink(output)));
}

void CryptoFile::encrypt(const std::string &input, std::string &output, unsigned char key[KEY_LEN])
{
    GCM_Final<AES, GCM_2K_Tables, true> gcm;
    gcm.SetKeyWithIV(key, 16, m_iv);
    StringSource(input, true, new AuthenticatedEncryptionFilter(gcm, new StringSink(output)));
}

void CryptoFile::decrypt(const char *buf, size_t length, std::string &output, unsigned char key[KEY_LEN])
{
    GCM_Final<AES, GCM_2K_Tables, false> gcm;
    gcm.SetKeyWithIV(key, 16, m_iv);
    ArraySource(
        (const unsigned char *)buf, length, true, new AuthenticatedDecryptionFilter(gcm, new StringSink(output)));
}

void CryptoFile::compress(std::istream &input, std::string &output)
{
    FileSource(input, true, new ZlibCompressor(new StringSink(output)));
}

void CryptoFile::decompress(const std::string &input, std::ostream &output)
{
    StringSource(input, true, new ZlibDecompressor(new FileSink(output)));
}

CryptoFile::CatRecord::CatRecord(const std::string &file, const FileInfo &info)
{
    memset(fileName, 0, FILE_NAME_LEN);
    memcpy(fileName, file.c_str(), file.length());
    memcpy(offset, &info.offset, OFFSET_LEN);
    memcpy(size, &info.size, SIZE_LEN);
    memcpy(key, info.key, KEY_LEN);
}

CryptoFile::FileInfo::FileInfo(const struct CatRecord &rec)
{
    offset = 0;
    memcpy(&offset, rec.offset, OFFSET_LEN);
    size = 0;
    memcpy(&size, rec.size, SIZE_LEN);
    memcpy(key, rec.key, KEY_LEN);
}

#ifndef _FILE_CORRUPT_H_
#define _FILE_CORRUPT_H_

#include <exception>

class FileCorrupt : public std::exception
{
};

#endif

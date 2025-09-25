#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "Strings.h"

namespace Verific {

    class FileSystem {
        public:
        static bool IsAbsolutePath(const char *);
        static char * DirectoryPath(const char *);
    };

}


#endif

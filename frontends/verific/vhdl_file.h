
#ifndef VHDL_FILE_H
#define VHDL_FILE_H

#include "DataBase.h"

namespace vhdl_file {

    typedef enum {
        VHDL_87,
        VHDL_93,
        VHDL_2K,
        VHDL_2008
    } vhdl_mode_t;


    Verific::VhdlLibrary * GetLibrary(const char *, int) { return nullptr; };
    void Reset() { };
    void SetDefaultLibraryPath(const char *) { };
    bool Analyze(const char *, const char *, vhdl_mode_t) { return false; };

}

#endif

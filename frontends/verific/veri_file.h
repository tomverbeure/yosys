
#ifndef VERI_FILE_H
#define VERI_FILE_H

#include "DataBase.h"

namespace veri_file {

    typedef enum {
        VERILOG_95,
        VERILOG_2K,
        SYSTEM_VERILOG,
        SYSTEM_VERILOG_2005,
        SYSTEM_VERILOG_2009
    } verilog_mode_t;

    typedef enum {
        MFCU
    } misc_def_t;

    Verific::VeriLibrary * GetLibrary(const char *, int) { return nullptr; };
    void Reset() { };
    void DefineCmdLineMacro(const char *, const char * = nullptr) { };
    void DefineMacro(const char *, const char * = nullptr) { };
    void UndefineMacro(const char *) { };
    void AddIncludeDir(const char *) { };
    void AddYDir(const char *) { };
    bool AnalyzeMultipleFiles(Verific::Array *, int, const char *, int) { return false; };
    Verific::VeriModule * GetModule(const char *) { return nullptr; };
    static void PrettyPrint(const char *, const char *, const char *) { };
    static void ElaborateAll(const char *) { };

}

#endif

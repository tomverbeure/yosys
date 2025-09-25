#ifndef ARRAY_H
#define ARRAY_H

namespace Verific {

    class Array {
        public:
        void InsertLast(VhdlLibrary *) { };
        void InsertLast(VhdlDesignUnit *) { };
        void InsertLast(VeriLibrary *) { };
        void InsertLast(VeriModule *) { };
        void InsertLast(const char *) { };
        void Insert(const char *) { };
        void Insert(Net *) { };
        int Size(){ return 0; };
    };

}

#endif

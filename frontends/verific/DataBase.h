#ifndef DATABASE_H
#define DATABASE_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <list>
#include <string>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

namespace Verific {

    typedef enum {
        VERIFIC_NONE,
        VERIFIC_ERROR,
        VERIFIC_WARNING,
        VERIFIC_IGNORE,
        VERIFIC_INFO,
        VERIFIC_COMMENT,
        VERIFIC_PROGRAM_ERROR
    } msg_type_t;

    typedef enum {
        PRIM_AND,
        PRIM_NAND,
        PRIM_OR,
        PRIM_NOR,
        PRIM_XOR,
        PRIM_XNOR,
        PRIM_BUF,
        PRIM_BUFIF1,
        PRIM_INV,
        PRIM_MUX,
        PRIM_TRI,
        PRIM_FADD,
        PRIM_DFF,
        PRIM_DFFRS,
        PRIM_DLATCH,
        PRIM_DLATCHRS,
        PRIM_PWR,
        PRIM_GND,
        PRIM_X,
        PRIM_Z,
        PRIM_HDL_ASSERTION,
        OPER_ADDER,
        OPER_MULTIPLIER,
        OPER_DIVIDER,
        OPER_MODULO,
        OPER_REMAINDER,
        OPER_SHIFT_LEFT,
        OPER_ENABLED_DECODER,
        OPER_SHIFT_RIGHT,
        OPER_REDUCE_AND,
        OPER_REDUCE_OR,
        OPER_REDUCE_XOR,
        OPER_REDUCE_XNOR,
        OPER_REDUCE_NOR,
        OPER_REDUCE_NAND,
        OPER_LESSTHAN,
        OPER_WIDE_AND,
        OPER_WIDE_OR,
        OPER_WIDE_XOR,
        OPER_WIDE_XNOR,
        OPER_DECODER,
        OPER_WIDE_BUF,
        OPER_WIDE_INV,
        OPER_MINUS,
        OPER_UMINUS,
        OPER_EQUAL,
        OPER_NEQUAL,
        OPER_WIDE_MUX,
        OPER_NTO1MUX,
        OPER_WIDE_NTO1MUX,
        OPER_SELECTOR,
        OPER_WIDE_SELECTOR,
        OPER_WIDE_TRI,
        OPER_WIDE_DFF,
        OPER_WIDE_DFFRS,
        OPER_WIDE_DLATCH,
        OPER_WIDE_DLATCHRS,
        OPER_WIDE_CASE_SELECT_BOX,
        OPER_READ_PORT,
        OPER_WRITE_PORT,
        OPER_CLOCKED_WRITE_PORT,

        PRIM_SVA_IMMEDIATE_ASSERT, PRIM_SVA_ASSERT, PRIM_SVA_COVER, PRIM_SVA_ASSUME,
        PRIM_SVA_EXPECT, PRIM_SVA_POSEDGE, PRIM_SVA_NOT, PRIM_SVA_FIRST_MATCH,
        PRIM_SVA_ENDED, PRIM_SVA_MATCHED, PRIM_SVA_CONSECUTIVE_REPEAT,
        PRIM_SVA_NON_CONSECUTIVE_REPEAT, PRIM_SVA_GOTO_REPEAT,
        PRIM_SVA_MATCH_ITEM_TRIGGER, PRIM_SVA_AND, PRIM_SVA_OR, PRIM_SVA_SEQ_AND,
        PRIM_SVA_SEQ_OR, PRIM_SVA_EVENT_OR, PRIM_SVA_OVERLAPPED_IMPLICATION,
        PRIM_SVA_NON_OVERLAPPED_IMPLICATION, PRIM_SVA_OVERLAPPED_FOLLOWED_BY,
        PRIM_SVA_NON_OVERLAPPED_FOLLOWED_BY, PRIM_SVA_INTERSECT, PRIM_SVA_THROUGHOUT,
        PRIM_SVA_WITHIN, PRIM_SVA_AT, PRIM_SVA_DISABLE_IFF, PRIM_SVA_SAMPLED,
        PRIM_SVA_ROSE, PRIM_SVA_FELL, PRIM_SVA_STABLE, PRIM_SVA_PAST,
        PRIM_SVA_MATCH_ITEM_ASSIGN, PRIM_SVA_SEQ_CONCAT, PRIM_SVA_IF,
        PRIM_SVA_RESTRICT, PRIM_SVA_TRIGGERED, PRIM_SVA_STRONG, PRIM_SVA_WEAK,
        PRIM_SVA_NEXTTIME, PRIM_SVA_S_NEXTTIME, PRIM_SVA_ALWAYS, PRIM_SVA_S_ALWAYS,
        PRIM_SVA_S_EVENTUALLY, PRIM_SVA_EVENTUALLY, PRIM_SVA_UNTIL, PRIM_SVA_S_UNTIL,
        PRIM_SVA_UNTIL_WITH, PRIM_SVA_S_UNTIL_WITH, PRIM_SVA_IMPLIES, PRIM_SVA_IFF,
        PRIM_SVA_ACCEPT_ON, PRIM_SVA_REJECT_ON, PRIM_SVA_SYNC_ACCEPT_ON,
        PRIM_SVA_SYNC_REJECT_ON, PRIM_SVA_GLOBAL_CLOCKING_DEF,
        PRIM_SVA_GLOBAL_CLOCKING_REF, PRIM_SVA_IMMEDIATE_ASSUME,
        PRIM_SVA_IMMEDIATE_COVER, OPER_SVA_SAMPLED, OPER_SVA_STABLE
    } inst_type_t;

    typedef enum {
        DIR_IN,
        DIR_OUT,
        DIR_INOUT,
        DIR_NONE
    } port_dir_t;

    static const std::string str_glb_empty = "str_glb_empty";
    static const char * char_glb_empty = "char_glb_empty";
    
    class Instance;
    class Netlist;
    class NetBus;
    class Net;
    class DesignObj;
    class PortBus;
    class Port;
    class PortRef;
    class VeriLibrary;
    class VeriModule;
    class VhdlLibrary;
    class VhdlDesignUnit;
    class Message;
    class TypeRange;
    class Oper;
    class OperWideCaseSelector;
    class Array;

    typedef bool linefile_type;

    // Need to redefine. Not a Verific thing.
    class InstanceList {
        public:
        Instance * GetLast() { return nullptr; };
    };

    class MapIter {
        public:
    };

    class SetIter {
    };

    class DesignObj {
        public:
        const char * Name() { return char_glb_empty; };
        const char * GetAttValue(const char *) { return char_glb_empty; };
        linefile_type Linefile() { return false; };
        bool IsFromVhdl() { return false; };
        bool GetAtt(const char *) { return false; };
    };

    class Att : public DesignObj  {
        public:
        const char * Key() { return char_glb_empty; }
        const char * Value() { return char_glb_empty; }
    };

    class TypeRange {
        public: 
        int LeftRangeBound() { return 0; };
        int RightRangeBound() { return 0; };
        long long GetScalarRangeLeftBound() { return 0; };
        long long GetScalarRangeRightBound() { return 0; };
        bool IsTypeScalar() { return false; };
        bool IsTypeEnum() { return false; };
        const char * GetTypeName() const { return nullptr; };
        MapIter * GetEnumIdMap() { return nullptr; };
    };

    class NetBus : public DesignObj {
        public:
        Net * ElementAtIndex(int) { return nullptr; };
        bool IsUp() { return false; };
        int LeftIndex() { return 0; };
        int RightIndex() { return 0; };
        int Size() { return 0; };
        bool IsUserDeclared() { return false; };
    };

    class Net : public DesignObj {
        public:
        Net(const char *) { };
        bool IsExternalTo(Netlist *) { return false; };
        Netlist * Owner() { return nullptr; };
        bool IsGnd() { return false; };
        bool IsPwr() { return false; };
        bool IsX() { return false; };
        bool IsRamNet() { return false; };
        bool IsUserDeclared() { return false; };
        bool IsConstant() { return false; };
        bool IsMultipleDriven() { return false; };
        int Size() { return 0; };
        char GetInitialValue() { return '0'; };
        const char * GetWideInitialValue() { return char_glb_empty; };
        Instance * Driver() { return nullptr; };
        NetBus * Bus() { return nullptr; };
        void Connect(Port *) { };
        TypeRange * GetOrigTypeRange() { return nullptr; };
        void BlastNet() { };
    };

    class PortBus : public DesignObj {
        public:
        unsigned int Size() { return 0; };
        Port * ElementAtIndex(int) { return nullptr; };
        int LeftIndex() { return 0; };
        int RightIndex() { return 0; };
        port_dir_t GetDir() { return DIR_INOUT; };
        bool IsUp() { return false; };
        int IndexOf(Port *) { return 0; };
    };

    class Port : public DesignObj {
        public:
        Port(const char *, port_dir_t) { };
        PortBus * Bus() { return nullptr; };
        port_dir_t GetDir() { return DIR_INOUT; };
        Net *GetNet() { return nullptr; };
    };

    class PortRef {
        public:
        Instance * GetInst() { return nullptr; };
        Port * GetPort() { return nullptr; };
        Net * GetNet() { return nullptr; };
    };

    class Instance : public DesignObj {
        public:
        Netlist * Owner() { return nullptr; };
        Netlist * View() { return nullptr; };
        unsigned int InputSize() { return 0; };
        unsigned int Input1Size() { return 0; };
        unsigned int Input2Size() { return 0; };
        unsigned int OutputSize() { return 0; };
        Net * GetInputBit(int) { return nullptr; }; 
        Net * GetInput1Bit(int) { return nullptr; }; 
        Net * GetInput2Bit(int) { return nullptr; }; 
        Net * GetOutputBit(int) { return nullptr; }; 
        Net * GetInput() { return nullptr; };
        Net * GetInput1() { return nullptr; };
        Net * GetInput2() { return nullptr; };
        Net * GetInput3() { return nullptr; };
        Net * GetOutput() { return nullptr; };
        Net * GetControl() { return nullptr; };
        Net * GetCin() { return nullptr; };
        Net * GetCout() { return nullptr; };
        Net * GetClock() { return nullptr; };
        Net * GetSet() { return nullptr; };
        Net * GetReset() { return nullptr; };
        Net * GetNet(int i){ return nullptr; };
        Net * GetNet(Port *){ return nullptr; };
        Net * GetAsyncCond(){ return nullptr; };
        Net * GetAsyncVal(){ return nullptr; };
        inst_type_t Type() { return PRIM_INV; };
        bool IsUserDeclared() { return false; };
        bool IsOperator() { return false; };
        bool IsPrimitive() { return false; };
        void Connect(Port *, Net *) { };
        void Disconnect(Port *) { };
    };

    class Netlist : public DesignObj {
        public:
        InstanceList *GetReferences() { return nullptr; };
        int NumOfRefs() { return 0; }
        std::string CellBaseName() { return str_glb_empty; }
        bool IsBlackBox() { return false; }
        bool IsEmptyBox() { return false; }
        PortBus * GetPortBus(const char *) { return nullptr; };
        Port * GetPort(const char *) { return nullptr; };
        bool IsSigned() { return false; };
        bool IsOperator() { return false; };
        bool IsPrimitive() { return false; };
        Instance * Owner() { return nullptr; };
        int IndexOf(Port *) { return 0; };
        void Add(Port *) { };
        void Add(Net *) { };
        void Flatten() { };
        TypeRange * GetTypeRange(const char *) { return nullptr; }
        static Netlist * PresentDesign() { return nullptr; };       // Current active design
        bool IsFromVerilog() { return true; };
        void RemoveDanglingLogic(int) { };
    };


    class LineFile {
        public:
        static const char * GetFileName(linefile_type linefile) 
        { 
            return "blah";
        }

        static int GetLineNo(linefile_type linefile) 
        { 
            return 0;
        }
    };

    class VhdlLibrary {
        public:
        VhdlDesignUnit * GetPrimUnit(const char *) { return nullptr; };
    };

    class VeriLibrary {
        public:
        VeriModule * GetModule(const char *) { return nullptr; };
    };

    class Libset {
        public:
        static void Reset() {};
    };

    class Message {
        public:
        static void SetConsoleOutput(int) { };
        static void SetMessageType(const char *, msg_type_t) { };
        static void RegisterCallBackMsg(void (&)(msg_type_t, const char*, linefile_type, const char *, va_list)) { };
        static const char * ReleaseString() { return nullptr; };
        static time_t ReleaseDate() { time_t raw_time; time(&raw_time); return raw_time; };
    };

    class RuntimeFlags {
        public:
        static void SetVar(const char *, int) { };
    };

    class Oper {
        public:
    };

    class OperWideCaseSelector : public Oper {
        public:
        unsigned GetNumBranches() { return 0; };
        unsigned GetNumConditions(int) { return 0; };
        void GetCondition(int, int, Array *, Array *) { };
    };
}

#define FOREACH_ATTRIBUTE(obj, mapIter, attr) attr = nullptr; for(int iii=0;iii<2;++iii)
#define FOREACH_PORT_OF_NETLIST(nl, mapIter, port) port = nullptr; for(int iii=0;iii<2;++iii)
#define FOREACH_PORTBUS_OF_NETLIST(nl, mapIter, portBus) portBus = nullptr; for(int iii=0;iii<2;++iii)
#define FOREACH_NET_OF_NETLIST(nl, mapIter, net) for(int iii=0;iii<2;++iii)
#define FOREACH_NETBUS_OF_NETLIST(nl, mapIter, netBus) netBus = nullptr; for(int iii=0;iii<2;++iii)
#define FOREACH_PORTREF_OF_NET(net, mapIter, portRef) portRef = nullptr; for(int iii=0;iii<2;++iii)
#define FOREACH_INSTANCE_OF_NETLIST(nl, mapIter, inst) inst = nullptr; for(int iii=0;iii<2;++iii)
#define FOREACH_PARAMETER_OF_NETLIST(nl, mapIter, param_name, param_value) for(int iii=0;iii<2;++iii)
#define FOREACH_PORTREF_OF_INST(inst, mapIter, portRef) portRef = nullptr; for(int iii=0;iii<2;++iii)
#define FOREACH_ARRAY_ITEM(array, i, item) item = nullptr; for(int iii=0;iii<2;++iii)
#define FOREACH_MAP_ITEM(map, i, key, value) *key = nullptr; *value=nullptr; for(int iii=0;iii<2;++iii)
#define FOREACH_PORT_OF_PORTBUS(portbus, setIter, port) port = nullptr; for(int iii=0;iii<2;++iii)
#define FOREACH_NET_OF_NETBUS(netbus, mapIter, net) net = nullptr; for(int iii=0;iii<2;++iii)

#pragma GCC diagnostic pop

#endif


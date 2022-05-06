#ifndef __SSA_H__
#define __SSA_H__

#include <unordered_map>

class SSAValue {
    public:
        int opcode;
        int id;
        SSAValue* op1, * op2;
        SSAValue* prev, * next;
        SSAValue* inCreationOrder;
        SSAValue* prevDomWithOpcode;
        SSAValue SSACreate(int opcode, SSAValue x, SSAValue y);


    private:
        //hash_map will be implemented, details still needed
        //std::unordered_map<>
        

        
};

#endif
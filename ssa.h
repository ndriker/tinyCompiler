#ifndef __SSA_H__
#define __SSA_H__

#include "basicBlock.h"

#include <unordered_map>

typedef struct {
    BasicBlock* currentJoinBlock;
    BasicBlock* currentFTBlock;
    bool isWhile;
} context;


// create enum for opcodes
// implement SSA Create

enum opcode {
	NEG = 0,        // negate x
	ADD = 1,        // add x y
	SUB = 2,        // sub x y
	MUL = 3,        // mul x y
	DIV = 4,        // div x y
	CMP = 5,        // compare x y
	ADDA = 6,       // add address
	LOAD = 7,       // load y
	STORE = 8,      // store y x
	PHI = 9,        // compute phi(x1, x2)
	END = 10,       // end of program
	BRA = 11,       // branch y
	BNE = 12,       // branch not equal x y
	BEQ = 13,       // branch equal x y
	BLE = 14,       // branch lte x y
	BLT = 15,       // branch lt x y
	BGE = 16,       // branch gte x y
	BGT = 17,       // branch gt x y
	read = 18,      // read
	write = 19,     // write x
	writeNL = 20,   // write new line
};


class SSAValue {
    public:
        static int maxID;
        opcode op;
        int id;
        SSAValue *operand1, *operand2;
        SSAValue *prev, *next;
        SSAValue *inCreationOrder; // store current loop head
        SSAValue *prevDomWithOpcode; // wtf is this
        SSAValue SSACreate(opcode operation, SSAValue* x, SSAValue* y);


    private:
        //hash_map will be implemented, details still needed
        //std::unordered_map<>



};

class SSA {
    public:
        SSA();
        void addSSAValue(SSAValue* newSSAVal);
        void addConditionalBlock();
        void addWhileBlock();

    private:
        SSAValue* instList;
        SSAValue* instTail;
        int instCounter;

        BasicBlock* bbHead;
        BasicBlock* bbTail;
        
        int bbCounter;
        context currentCtx;
        
};

#endif
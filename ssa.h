#ifndef __SSA_H__
#define __SSA_H__

#include "token.h"
#include <unordered_map>

// create enum for opcodes
// implement SSA Create

enum opcode {
	NEG = 0,        // negate x
	ADDOP = 1,      // add x y
	SUBOP = 2,      // sub x y
	MULOP = 3,      // mul x y
	DIVOP = 4,      // div x y
	CMP = 5,        // compare x y
	ADDA = 6,       // add address
	LOAD = 7,       // load y
	STORE = 8,      // store y x
	PHI = 9,        // compute phi(x1, x2), x1 if from left side, x2 if from right side
	END = 10,       // end of program
	BRA = 11,       // branch y
	BNE = 12,       // branch not equal x y
	BEQ = 13,       // branch equal x y
	BLE = 14,       // branch lte x y
	BLT = 15,       // branch lt x y
	BGE = 16,       // branch gte x y
	BGT = 17,       // branch gt x y
    CONST = 18,     // constant
	NOP = 19,		// no operation
	read = 20,      // read
	write = 21,     // write x
	writeNL = 22,   // write new line
};


class SSAValue {
    public:
        opcode op;
        int constValue;
        int id;
        SSAValue *operand1, *operand2;
        SSAValue *prev, *next;
        SSAValue *inCreationOrder; // store current loop head
        SSAValue *prevDomWithOpcode; // wtf is this

		int bbID;
		bool firstInstOfBB;

		// ssa value debugging functions
		std::string getTextForEnum(int enumVal);
		std::string formatOperand(SSAValue* operand);
        void instRepr(); // print representation of each ssa value
		std::string instCFGRepr();



    private:
        //hash_map will be implemented, details still needed
        //std::unordered_map<>
		std::string opcodeEnumStrings[23] = {
			"NEG",        
			"ADDOP",        
			"SUBOP",        
			"MULOP",        
			"DIVOP",        
			"CMP",        
			"ADDA",       
			"LOAD",       
			"STORE",      
			"PHI",        
			"END",       
			"BRA",       
			"BNE",       
			"BEQ",       
			"BLE",       
			"BLT",       
			"BGE",       
			"BGT",       
			"CONST",
			"NOP",
			"read",      
			"write",     
			"writeNL"   
		};




};

class SSA {
    public:
        SSA();
        void addSSAValue(SSAValue* newSSAVal);
        SSAValue* SSACreate(opcode operation, SSAValue* x, SSAValue* y);
		void SSACreate(opcode operation, SSAValue* y);
		SSAValue* SSACreateWhilePhi(SSAValue* x, SSAValue* y);
        SSAValue* SSACreateConst(int constVal);
		SSAValue* SSACreateNop();

		void updateNop(SSAValue* nopInst);
		
		int getTailID();
		SSAValue* getTail();
		void setInstTail(SSAValue* newTail);

		opcode convertBr(tokenType type);

        // symTable function
        void enterScope();
        std::unordered_map<std::string, SSAValue*> exitScope();
        void addSymbol(std::string name, SSAValue* val);
        SSAValue* findSymbol(std::string name);

        // constTable functions
        void addConst(int constVal, SSAValue* constSSAVal);
        SSAValue* findConst(int constVal);



        // ssa debugging functions
        void printSSA();
		void printSymTable();
		void printConstTable();

		void generateDotLang();
    private:
        static int maxID; // current number of SSAValues created
		static std::unordered_map<tokenType, opcode> brOpConversions;

        SSAValue* instList; // pointer to head of instruction list
        SSAValue* instTail; // pointer to tail of instruction list
        int instListLength;

		int scopeDepth;

        std::vector<std::unordered_map<std::string, SSAValue*>> symTable;
		std::vector<std::unordered_map<std::string, SSAValue*>> symTableCopy;

        std::unordered_map<int, SSAValue*> constTable;



        
};

class BasicBlock {
	public:
		void setHead(SSAValue* head);
		void setTail(SSAValue* tail);
		void setBlockLabel(std::string label);
	private:
		std::string blockLabel;
		SSAValue* headInst;
		SSAValue* tailInst;
		static int maxID;
};


class BBEdge {
	public:
		BBEdge(int from);
		void setToBlockID(int to);
		int getFrom();
		int getTo();
	private:
		int fromBlockID;
		int toBlockID;
		bool edgeComplete;
};


#endif
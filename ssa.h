#ifndef __SSA_H__
#define __SSA_H__

#include "token.h"
#include <unordered_map>
#include <tuple>

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
		std::string op1Name, op2Name;
        SSAValue *prev, *next;
        SSAValue *inCreationOrder; 
        SSAValue *prevDomWithOpcode; // wtf is this


		int bbID;
		bool firstInstOfBB;

		void setOpNames(std::string operand1Name, std::string operand2Name);

		// ssa value debugging functions
		std::string getTextForEnum(int enumVal);
		std::string formatOperand(SSAValue* operand);

		std::string getType();
		std::string getNameType();


        void instRepr(); // print representation of each ssa value
		void instReprWNames();
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

class BasicBlock {
	// we want a map between branch-to inst ids and branch bbs
	public:
		std::string bbRepr();
		SSAValue* getHead();
		SSAValue* getTail();
		int getID();
		int id;
		SSAValue* head;
		SSAValue* tail;
		BasicBlock* fallThrough;
		BasicBlock* branch;

};

class BBEdge {
public:
	BBEdge(BasicBlock* fromBlock, BasicBlock* toBlock, std::string edgeType);
	BasicBlock* getFrom();
	BasicBlock* getTo();
	std::string getType();
	std::string bbEdgeRepr();
private:
	BasicBlock* from;
	BasicBlock* to;
	std::string type;
};


class SSA {
    public:
        SSA();
        void addSSAValue(SSAValue* newSSAVal);
		void addSSAConst(SSAValue* newSSAVal);
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

		// inorder functions
		void addInOrder(SSAValue* newInst);
		SSAValue* findPrevDomWithOpcode(opcode operation);

        // ssa debugging functions
		std::string outputSSA();
        void printSSA();
		void printSymTable();
		void printConstTable();
		std::string reprBasicBlocks(BasicBlock* head);

	    // basic block functions
		BasicBlock* createContext();
		BasicBlock* createBlock();
		void initBlock(BasicBlock* blockToInit);
		void addInstToBB(SSAValue* inst);
		void addInstToConstBB(SSAValue* inst);
		void connectFT(BasicBlock* from, BasicBlock* to);
		void connectBR(BasicBlock* from, BasicBlock* to);
		BasicBlock* getContext();
		void setContext(BasicBlock* ctx);

		std::string genBBStart(int bbID);
		std::tuple<std::vector<BasicBlock*>, std::vector<BBEdge*>> genBasicBlocks();
		void gen();
		//void generateDotLang();

		void reset();
    private:
        static int maxID; // current number of SSAValues created
		static int numConsts;
		static int maxBlockID;
		static std::unordered_map<tokenType, opcode> brOpConversions;

        SSAValue* instList; // pointer to head of instruction list
        SSAValue* instTail; // pointer to tail of instruction list
        int instListLength;

		int scopeDepth;

        std::vector<std::unordered_map<std::string, SSAValue*>> symTable;
		std::vector<std::unordered_map<std::string, SSAValue*>> symTableCopy;

		std::vector<std::vector<SSAValue*>> inOrder;

        std::unordered_map<int, SSAValue*> constTable;

		std::vector<BasicBlock*> basicBlocks;

		BasicBlock* bbListHead;
		BasicBlock* constBlock;
		BasicBlock* context;

        
};








#endif
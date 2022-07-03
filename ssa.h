#ifndef __SSA_H__
#define __SSA_H__

#include "token.h"
#include <unordered_map>
#include <set>
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
	argAssign = 23,
	call = 24,
	ret = 25
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
		bool eliminated;
		SSAValue* eliminatedBy;
		std::string argName;
		std::vector<SSAValue*> callArgs;
		std::vector<std::string> formalParams;

		bool isVoidCall;


		int bbID;
		bool firstInstOfBB;

		bool deadCode;

		void setOpNames(std::string operand1Name, std::string operand2Name);

		// ssa value debugging functions
		std::string getTextForEnum(int enumVal);
		std::string formatOperand(SSAValue* operand);

		std::string getType();
		std::string getNameType();

		std::string callRepr();
        void instRepr(); // print representation of each ssa value
		void instReprWNames();
		std::string instCFGRepr();
		std::string elimRepr();




    private:
        //hash_map will be implemented, details still needed
        //std::unordered_map<>
		std::string opcodeEnumStrings[26] = {
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
			"writeNL",
			"argAssign",
			"call",
			"return"
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

		BasicBlock* loopBackFrom;
		BasicBlock* fallThroughFrom;
		BasicBlock* branchFrom;

		BasicBlock* loop;

		BasicBlock* fallThrough;
		BasicBlock* branch;
		bool addToHead;
		bool headAdded;
		std::string joinType;
		std::string splitType;
		std::string funcName;
		BasicBlock* dom;
		bool visited;
		bool ifThenJoinBlock; // technically if then else join block
		bool alreadyConnectedBranch;
		bool alreadyConnectedFT;
		bool alreadyConnected;
		int numVisits;

		void unionLiveRanges(BasicBlock* toUnionFrom, std::string edgeType);

		std::set<SSAValue*> liveRanges;
		std::vector<SSAValue*> phis;

		std::string conditionalBlockType;

};


class SSA {
    public:
        SSA(std::string fName);
        void addSSAValue(SSAValue* newSSAVal);
		void addSSAConst(SSAValue* newSSAVal);
        SSAValue* SSACreate(opcode operation, SSAValue* x, SSAValue* y);
		SSAValue* SSACreate(opcode operation, SSAValue* y);
		SSAValue* SSACreateWhilePhi(SSAValue* x, SSAValue* y);
        SSAValue* SSACreateConst(int constVal);
		SSAValue* SSACreateNop();
		SSAValue* SSACreateArgAssign(std::string argName);
		SSAValue* SSACreateCall(std::string funcName, std::vector<SSAValue*> cArgs, std::vector<std::string> fParams);


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


		// varDeclFunctions
		void addToVarDecl(std::string varName);
		bool checkVarDeclList(std::string varName);
		// inorder functions
		void addInOrder(SSAValue* newInst);
		SSAValue* findPrevDomWithOpcode(opcode operation);

        // ssa debugging functions
		std::string outputSSA();
		void removeElimInsts();
        void printSSA();
		void printSymTable();
		void printConstTable();
		void printVarDeclList();
		std::string reprBasicBlocks(BasicBlock* head);


	    // basic block functions
		BasicBlock* createContext();
		BasicBlock* createBlock();
		void setJoinType(BasicBlock* block, std::string type);
		void initBlock(BasicBlock* blockToInit);
		void addInstToBB(SSAValue* inst);
		void addInstToConstBB(SSAValue* inst);
		void connectFT(BasicBlock* from, BasicBlock* to);
		void connectBR(BasicBlock* from, BasicBlock* to);
		void connectLoop(BasicBlock* from, BasicBlock* to);
		BasicBlock* findBBWithID(int id);


		BasicBlock* getContext();
		void setContext(BasicBlock* ctx, bool addToHead=false);

		void correctBasicBlockIssues();

		void traverseBasicBlocks(BasicBlock* startBlock);
		BasicBlock* getBBListHead();
		BasicBlock* getBBTail();

		std::string genBBStart(int bbID);
		
		bool getInWhile();
		void setInWhile(bool value);
		
		void gen();
		//void generateDotLang();

		void reset();
		
		void checkOperands(SSAValue* currentInst);
		void generateLiveRanges(std::set<SSAValue*>& liveRanges, std::vector<SSAValue*>& phis, SSAValue* instTail, SSAValue* stopAt);
		void printLiveRanges();
		void handleTraverseStep(BasicBlock* bb);


    private:
        int maxID; // current number of SSAValues created
		int numConsts;
		int maxBlockID;
		static std::unordered_map<tokenType, opcode> brOpConversions;

		bool inWhile;
        SSAValue* instList; // pointer to head of instruction list
        SSAValue* instTail; // pointer to tail of instruction list
        int instListLength;

		int scopeDepth;

        std::vector<std::unordered_map<std::string, SSAValue*>> symTable;
		std::vector<std::unordered_map<std::string, SSAValue*>> symTableCopy;

		std::vector<std::vector<SSAValue*>> inOrder;

        std::unordered_map<int, SSAValue*> constTable;

		std::vector<BasicBlock*> basicBlocks;

		std::vector<std::string> varDeclList;

		BasicBlock* bbListHead;
		BasicBlock* constBlock;
		BasicBlock* context;

		std::string funcName;

		std::unordered_map<SSAValue*, std::set<SSAValue*>> iGraph;



        
};








#endif
#include "ssa.h"
#include "token.h"
#include <stdexcept>
#include <iostream>
#include <string>
#include <set>
#include <queue>
#include <stack>

// ssa value class

std::string SSAValue::getTextForEnum(int enumVal) {
	return opcodeEnumStrings[enumVal];
}

std::string SSAValue::formatOperand(SSAValue* operand) {
	if (operand->op == NOP && operand->label != "") {
		return "(" + operand->label + ")";
	} else {
		return "(" + std::to_string(operand->id) + ")";
	}
}

std::string SSAValue::formatRegOperand(SSAValue* operand) {
	if (operand->regNum != 0) {
		return "(R" + std::to_string(operand->regNum) + ")";
	} else {
		return formatOperand(operand);
	}
}

std::string SSAValue::callRepr() {
	std::string output;
	if (regNum != 0) {
		output = "R" + std::to_string(regNum) + "= call " + argName + "(";
	} else {

		output = "call " + argName + "(";
	}
	for (int i = 0; i < formalParams.size(); i++) {
		SSAValue* callArg = callArgs.at(i);
		std::string id;
		if (callArg->regNum != 0) {
			id = "R" + std::to_string(callArg->regNum);
		} else {
			id = std::to_string(callArg->id);
		}
		output += formalParams.at(i) + " : (" + id + "), ";
	}
	output.pop_back();
	output.pop_back();
	output += ")";
	return output;
}


std::string SSAValue::moveRepr() {
	std::string output = "MOVE R" + std::to_string(regToMoveFrom) + ", R" + std::to_string(regToMoveTo);
	return output;
}

std::string SSAValue::moveConstRepr() {
	std::string output = "MOVE #" + std::to_string(operand1->constValue) + ", R" + std::to_string(regToMoveTo);
	return output;
}
void SSAValue::instRepr() {
	if (op == call) {
		std::cout << callRepr() << std::endl;
	} else if (op == argAssign) {
		std::cout << id << ": arg " << argName << std::endl;
	} else if (op == CONST) {
		std::cout << id << ": CONST #" << constValue << std::endl;
	} else if (op == BRA || op == write || op == ret) {
		std::cout << id << ": " << getTextForEnum(op) << formatOperand(operand1) << std::endl;
	} else if (op == NOP || op == read || op == writeNL) {
		std::cout << id << ": " << getTextForEnum(op) << std::endl;
	} else {
		std::cout << id << ": " << getTextForEnum(op) << formatOperand(operand1) << formatOperand(operand2) << std::endl;
	}
}


std::string SSAValue::getNameType() {
	std::string info = "; ";
	if (operand1 != nullptr) {
		info += "lArg: " + op1Name;
	}
	if (operand2 != nullptr) {
		info += ", rArg: " + op2Name;
	}
	return info;
}

void SSAValue::instReprWNames() {
	if (op == call) {
		std::cout << callRepr() << std::endl;
	} else if (op == argAssign) {
		std::cout << id << ": arg " << argName << std::endl;
	} else if (op == CONST) {
		std::cout << id << ": CONST #" << constValue << getNameType() << std::endl;
	} else if (op == BRA || op == write || op == ret) {
		std::cout << id << ": " << getTextForEnum(op) << formatOperand(operand1) << getNameType() << std::endl;
	} else if (op == NOP || op == read || op == writeNL) {
		std::cout << id << ": " << getTextForEnum(op) << getNameType() << std::endl;
	} else {
		if (prevDomWithOpcode != nullptr) {
			std::cout << id << ": " << getTextForEnum(op) << formatOperand(operand1) << formatOperand(operand2) << getNameType() << ", " << prevDomWithOpcode->instCFGRepr() << std::endl;
		} else {
			std::cout << id << ": " << getTextForEnum(op) << formatOperand(operand1) << formatOperand(operand2) << getNameType() << ", " << "No Dom Inst with Opcode" << std::endl;
		}
	}
}

std::string SSAValue::elimRepr() {
	return std::to_string(id) + ": \\<eliminated by " +
		std::to_string(eliminatedBy->id) + ": " +
		getTextForEnum(eliminatedBy->op) +
		formatOperand(eliminatedBy->operand1) +
		formatOperand(eliminatedBy->operand2) + "\\>";
}


std::string SSAValue::instCFGRepr() {
	std::string output = "";
	if (op == call) {
		output = callRepr();
	} else if (op == argAssign) {
		output = std::to_string(id) + ": arg " + argName;
	} else if (eliminated) {
		output = elimRepr();
	} else if (op == CONST) {
		output = std::to_string(id) + ": CONST #" + std::to_string(constValue);
	} else if (op == BRA || op == write || op == ret) {
		output = std::to_string(id) + ": " + getTextForEnum(op) + formatOperand(operand1);
	} else if (op == NOP || op == read || op == writeNL) {
		output = std::to_string(id) + ": " + getTextForEnum(op);
	} else {
		output = std::to_string(id) + ": " + getTextForEnum(op) + formatOperand(operand1) + formatOperand(operand2);
		
	}
	return output;
}

std::string SSAValue::instCFGRegRepr() {
	std::string output = "";

	if (op == moveConst) {
		output = moveConstRepr();
	} else if (op == move) {
		output = moveRepr();
	} else if (op == PHI) {
		output = "";
	} else if (op == call) {
		output = callRepr();
	} else if (op == argAssign) {
		output = "R" + std::to_string(regNum) + "= arg " + argName;
	} else if (eliminated) {
		output = elimRepr();
	} else if (op == CONST) {
		output = std::to_string(id) + ": CONST #" + std::to_string(constValue);
	} else if (op == BRA || op == write || op == ret) {
		output = getTextForEnum(op) + formatRegOperand(operand1);
	} else if (op == NOP || op == writeNL) {
		output = label + ":";
	} else if (op == read) {
		output = "R" + std::to_string(regNum) + "= " + getTextForEnum(op);
	} else {
		if (op >= BNE && op <= BGT) {
			output = std::to_string(id) + ": " + getTextForEnum(op) + formatRegOperand(operand1) + formatRegOperand(operand2);
		} else {
			output = "R" + std::to_string(regNum) + "= " + getTextForEnum(op) + formatRegOperand(operand1) + formatRegOperand(operand2);
		}

	}
	return output;
}

void SSAValue::setOpNames(std::string operand1Name, std::string operand2Name) {
	op1Name = operand1Name;
	op2Name = operand2Name;
}

// ssa class



std::unordered_map<tokenType, opcode> SSA::brOpConversions = {
	{NEQ, BEQ},
	{EQ, BNE},
	{LE, BGT},
	{LT, BGE},
	{GE, BLT},
	{GT, BLE}
};

opcode SSA::convertBr(tokenType type) {
	return brOpConversions[type];
}

SSAValue* SSA::SSACreate(opcode operation, SSAValue* x, SSAValue* y) {
	SSAValue* prevDomWithOpcode = nullptr;
	SSAValue* elimBy = nullptr;
	if (operation >= ADDOP && operation <= DIVOP) {
	//	// these ops can be involved in CSE
	//	// search in symbol table for dominating inst with same opcode
	//	// then search up the linked list looking for an inst that has same params
		prevDomWithOpcode = findPrevDomWithOpcode(operation);

		SSAValue* iter = prevDomWithOpcode;
		while (iter != nullptr) {
			std::cout << "SSACreate prevDomWithOpcode" << iter->instCFGRepr() << std::endl;
			if (!inWhile && iter->operand1 == x && iter->operand2 == y) {

				// found common subexpression
				elimBy = iter;
			}
			iter = iter->prevDomWithOpcode;
		}

	}
	SSAValue* result = new SSAValue();
	result->id = maxID++;
	result->op = operation;
	result->operand1 = x;
	result->operand2 = y;
	result->prevDomWithOpcode = prevDomWithOpcode;
	addSSAValue(result);
	addInOrder(result);
	if (elimBy != nullptr) {
		result->eliminated = true;
		result->eliminatedBy = elimBy;
		return elimBy;
	}
	return result;

}

SSAValue* SSA::SSACreateMove(int currentReg, int moveToReg) {
	SSAValue* moveInstr = new SSAValue();
	moveInstr->id = maxID++;
	moveInstr->op = move;
	moveInstr->regToMoveFrom = currentReg;
	moveInstr->regToMoveTo = moveToReg;
	return moveInstr;
	
}

SSAValue* SSA::SSACreateConstMove(SSAValue* constInstr, int moveToReg) {
	SSAValue* moveInstr = new SSAValue();
	moveInstr->id = maxID++;
	moveInstr->op = moveConst;
	moveInstr->operand1 = constInstr;
	moveInstr->regToMoveTo = moveToReg;
	return moveInstr;
}
SSAValue* SSA::SSACreateWhilePhi(SSAValue* x, SSAValue* y) {
	SSAValue* result = new SSAValue();
	result->id = maxID++;
	result->op = PHI;
	result->operand1 = x;
	result->operand2 = y;
	addInOrder(result);
	return result;
}

SSAValue* SSA::SSACreate(opcode operation, SSAValue* y) {
	SSAValue* result = new SSAValue();
	result->id = maxID++;
	result->op = operation;
	result->operand1 = y;
	addInOrder(result);
	addSSAValue(result);
	return result;
}

SSAValue* SSA::SSACreateArgAssign(std::string argName) {
	SSAValue* result = new SSAValue();
	result->id = maxID++;
	result->op = argAssign;
	result->argName = argName;
	addInOrder(result);
	addSSAValue(result);
	return result;
}

SSAValue* SSA::SSACreateCall(std::string funcName, std::vector<SSAValue*> cArgs, std::vector<std::string> fParams) {
	SSAValue* result = new SSAValue();
	result->id = maxID++;
	result->op = call;
	result->argName = funcName;
	result->callArgs = cArgs;
	result->formalParams = fParams;
	addInOrder(result);
	addSSAValue(result);
	return result;
}

SSAValue* SSA::SSACreateConst(int constVal) {
	std::cout << "Creating const" << std::endl;
	SSAValue* val = findConst(constVal);
	if (val == nullptr) {
		std::cout << "Val was nullptr" << std::endl;
		SSAValue* ssaConst = new SSAValue();
		ssaConst->op = CONST;
		numConsts += 1;
		ssaConst->id = -numConsts;
		ssaConst->constValue = constVal;
		addInOrder(ssaConst);
		addConst(constVal, ssaConst);
		addSSAConst(ssaConst);
		return ssaConst;
	}
	return val;
}

SSAValue* SSA::SSACreateNop() {
	SSAValue* result = new SSAValue();
	result->op = NOP;
	return result;
}


void SSA::updateNop(SSAValue* nopInst) {
	// set other fields of nop ssavalue
	nopInst->id = maxID++;
	addSSAValue(nopInst);
	addInOrder(nopInst);

}	

void SSA::addInOrder(SSAValue* newInst) {
	inOrder.back().push_back(newInst);
}

SSAValue* SSA::findPrevDomWithOpcode(opcode operation) {
	// find previous dominating instruction with same opcode
	for (int i = inOrder.size() - 1; i >= 0; i--) {

		std::vector<SSAValue*> inner = inOrder.at(i);
		//std::cout << operation << std::endl;
		for (int j = inner.size() - 1; j >= 0; j--) {
			SSAValue* item = inner.at(j);
			if (item->op == operation) {
				return item;
			}
		}
	}
	return nullptr;
}
void SSA::setInWhile(bool value) {
	inWhile = value;
}

bool SSA::getInWhile() {
	return inWhile;
}

int SSA::getTailID() {
	return instTail->id;
}

SSAValue* SSA::getTail() {
	return instTail;
}

void SSA::setInstTail(SSAValue* newTail) {
	instTail = newTail;
}

SSA::SSA(std::string fName) {
	funcName = fName;
	maxID = 0;
	numConsts = 0;
	maxBlockID = 0;
	instListLength = 0;
	instTail = new SSAValue();
	scopeDepth = 0;
	createContext();
	constBlock = new BasicBlock();
	constBlock->funcName = funcName;
	constBlock->id = -1;
	//basicBlocks.push_back(context);
	enterScope(); // built-in functions and computation var decls
}


void SSA::addSSAValue(SSAValue* newSSAVal) {
	if (instListLength == 0) {
		instList = newSSAVal;
		instTail = newSSAVal;

	} else {
		instTail->next = newSSAVal;
		newSSAVal->prev = instTail;
		instTail = newSSAVal;

	}
	addInstToBB(newSSAVal);
	instListLength = instListLength + 1;
}
void SSA::addSSAConst(SSAValue* newSSAVal) {
	if (instListLength == 0) {
		instList = newSSAVal;
		instTail = newSSAVal;

	} else {
		newSSAVal->next = instList;
		instList->prev = newSSAVal;
		instList = newSSAVal;

	}
	addInstToConstBB(newSSAVal);
	instListLength = instListLength + 1;
}
// symbolTable functions

void SSA::enterScope() {
	symTable.push_back(std::unordered_map<std::string, SSAValue*>());
	symTableCopy.push_back(std::unordered_map<std::string, SSAValue*>());

	inOrder.push_back(std::vector<SSAValue*>());

	scopeDepth = scopeDepth + 1;
}

std::unordered_map<std::string, SSAValue*> SSA::exitScope() {
	std::unordered_map<std::string, SSAValue*> lastScope = symTable.back();
	symTable.pop_back();
	inOrder.pop_back();

	scopeDepth = scopeDepth - 1;
	return lastScope;
}

void SSA::addSymbol(std::string name, SSAValue* val) {
	if (checkVarDeclList(name)) {
		symTable.back().insert_or_assign(name, val);
		symTableCopy.back().insert_or_assign(name, val);
	} else {
		std::cout << "Parser Error: Var Decl not found for " << name << std::endl;
	}
}

SSAValue* SSA::findSymbol(std::string name) {
	if (checkVarDeclList(name)) {
		for (int i = symTable.size() - 1; i >= 0; i--) {
			std::unordered_map<std::string, SSAValue*> scopeMap = symTable[i];

			try {
				SSAValue* val = scopeMap.at(name);
				return val;
			}
			catch (std::out_of_range& oor) {
				// intentionally left blank
			}
		}
		// if variable is not defined when attempting access
		//	set var value to 0
		SSAValue* constZero = SSACreateConst(0);
		return constZero;
	} else {
		std::cout << "Parser Error: Var Decl not found for " << name << std::endl;
		return nullptr;
	}


}

// constTable functions

void SSA::addConst(int constVal, SSAValue* constSSAVal) {
	constTable.insert({ constVal, constSSAVal });

}

SSAValue* SSA::findConst(int constVal) {
	std::cout << "Const Table" << std::endl;
	for (auto kv : constTable) {
		std::cout << kv.first << " " << kv.second->instCFGRepr() << std::endl;
	}
	std::cout << "end of const table" << std::endl;
	std::cout << "val is " << constVal << std::endl;
	try {
		SSAValue* val = constTable.at(constVal);
		return val;
	} catch (std::out_of_range& oor) {
		return nullptr;
	}

}


// var decl functions
void SSA::addToVarDecl(std::string varName) {
	varDeclList.push_back(varName);
}

bool SSA::checkVarDeclList(std::string varName) {
	for (std::string var : varDeclList) {
		if (var == varName) {
			return true;
		}
	}
	return false;
}
// ssa debugging functions

void SSA::removeElimInsts() {
	SSAValue* current = instList;
	while (current != nullptr) {
		if (current->eliminated) {
			SSAValue* beforeCurrent = current->prev;
			SSAValue* afterCurrent = current->next;

			beforeCurrent->next = afterCurrent;
			if (afterCurrent != nullptr) {

				afterCurrent->prev = beforeCurrent;
			}
		}
		current = current->next;
	}
}
std::string SSA::outputSSA() {
	removeElimInsts();
	std::string output = "";
	SSAValue* current = instList;
	while (current != nullptr) {
		output += current->instCFGRepr() + "\n";
		current = current->next;
	}
	return output;
}


void SSA::printSSA() {
	removeElimInsts();

	SSAValue* current = instList;
	while (current != nullptr) {
		current->instReprWNames();

		current = current->next;
	}
}

void SSA::printSymTable() {
	std::cout << "Symbol       ID       Scope" << std::endl;
	for (int i = 0; i < symTableCopy.size(); i++) {
		for (auto kv : symTableCopy[i]) {
			std::cout << kv.first << "            (" << kv.second->id << ")      "<< i << std::endl;
		}
	}

}

void SSA::printConstTable() {
	std::cout << "Consts       ID" << std::endl;
	for (auto kv : constTable) {
		std::cout << kv.first << "            (" << kv.second->id << ")" << std::endl;
	}
}

void SSA::printVarDeclList() {
	std::cout << "Var Declarations" << std::endl;
	for (std::string name : varDeclList) {
		std::cout << name << std::endl;
	}
}

std::string SSA::reprBasicBlocks(BasicBlock* head, bool printRegs) {

	if (constBlock->fallThrough == nullptr) {
		connectFT(constBlock, bbListHead);
		basicBlocks.push_back(constBlock);
	}
	std::string out_string;
	for (BasicBlock* bb : basicBlocks) {
		out_string += bb->bbRepr(printRegs) + "\n";
	}
	return out_string;
}



void SSA::gen(bool printRegs) {

	std::cout << reprBasicBlocks(bbListHead, printRegs) << std::endl;
}

BasicBlock* SSA::createContext() {
	BasicBlock* newBlock = new BasicBlock();
	newBlock->id = maxBlockID++;
	if (bbListHead == nullptr) {
		bbListHead = newBlock;
	}
	newBlock->funcName = funcName;
	context = newBlock;
	basicBlocks.push_back(context);
	return newBlock;
}

void SSA::addInstToBB(SSAValue* inst) {
	if (context->head == nullptr) {
		context->head = inst;
	}

	if (context->addToHead) {
		if (context->head->op == CMP) {
			context->head = inst;
			context->addToHead = false;
			context->headAdded = true;
		}
	} else {
		if (!context->headAdded) {
			context->tail = inst;

		}
	}
	inst->containingBB = context;
}

void SSA::addInstToConstBB(SSAValue* inst) {
	if (constBlock->tail == nullptr) {
		constBlock->tail = inst;
	}
	constBlock->head = inst;

}

void SSA::connectFT(BasicBlock* from, BasicBlock* to) {

	if (context->fallThrough != nullptr) {
		std::cout << "FT" << context->id << " " << context->fallThrough->id << "BAD" << std::endl;
	}
	if (from->fallThrough == nullptr) {

		from->fallThrough = to;
		to->fallThroughFrom = from;
	}
}

void SSA::connectBR(BasicBlock* from, BasicBlock* to) {
	if (context->branch != nullptr) {
		std::cout << "BR" << context->branch->id << "BAD" << std::endl;
	}
	if (from != nullptr && from->branch == nullptr) {
		from->branch = to;
		to->branchFrom = from;
	}
}

void SSA::connectLoop(BasicBlock* from, BasicBlock* to) {
	from->loop = to;
	to->loopBackFrom = from;
}

BasicBlock* SSA::getContext() {
	return context;
}

void SSA::setContext(BasicBlock* ctx, bool addToHead) {
	context = ctx;
	context->addToHead = addToHead;
	context->headAdded = false;
}

BasicBlock* SSA::createBlock() {
	BasicBlock* bb = new BasicBlock();
	bb->funcName = funcName;
	return bb;
}
void SSA::setJoinType(BasicBlock* block, std::string type) {
	block->joinType = type;
}

void SSA::initBlock(BasicBlock* blockToInit) {
	blockToInit->id = maxBlockID++;
	basicBlocks.push_back(blockToInit);

}



SSAValue* BasicBlock::getHead() {
	return head;
}


SSAValue* BasicBlock::getTail() {
	return tail;
}


int BasicBlock::getID() {
	return id;
}


std::string BasicBlock::bbRepr(bool printRegs) {
	SSAValue* current = head;

	SSAValue* stopAt;

	if (tail == nullptr) {
		stopAt= head;
	} else {
		stopAt = tail->next;
	}
	std::string bbIDStr = std::to_string(id);
	if (id == -1) {
		bbIDStr = "CONST";
	}
	std::string bbString = funcName + bbIDStr + "[shape=record, label=\" < b > " + funcName + bbIDStr + " | {";
	while (current!= nullptr && current != stopAt) {
		if (printRegs) {
			bbString = bbString + current->instCFGRegRepr() + "|";
		} else {
			bbString = bbString + current->instCFGRepr() + "|";

		}
		current = current->next;
	}
	bbString.pop_back();
	bbString = bbString + "}\"];";
	if (fallThrough != nullptr) {
		std::string fromIDStr = std::to_string(id);
		std::string type = "FT";
		if (id == -1) {
			fromIDStr = "CONST";
			type = "";
		}
		std::string toIDStr = std::to_string(fallThrough->id);

		std::string out = funcName + fromIDStr + ":s -> " + funcName + toIDStr + ":n [label=\"" + type + "\"];";
		bbString += "\n" + out;
	}
	if (branch != nullptr) {
		std::string fromIDStr = std::to_string(id);
		std::string toIDStr = std::to_string(branch->id);
		std::string out = funcName + fromIDStr + ":s -> " + funcName + toIDStr + ":n [label=\"" + "BR" + "\"];";
		bbString += "\n" + out;
	}
	if (loop != nullptr) {
		std::string fromIDStr = std::to_string(id);
		std::string toIDStr = std::to_string(loop->id);
		std::string out = funcName + fromIDStr + ":s -> " + funcName + toIDStr + ":n [label=\"" + "BRLoop" + "\"];";
		bbString += "\n" + out;
	}
	if (dom != nullptr) {
		std::string toIDStr = std::to_string(id);
		std::string fromIDStr = std::to_string(dom->id);
		std::string out = funcName + fromIDStr + ":b ->" + funcName + toIDStr + ":b [color=blue, style=dotted, label=\"dom\"] ";
		bbString += "\n" + out;
	}
	return bbString;
}


void SSA::reset() {
	maxID = 0;
	instList = nullptr;
	instTail = nullptr;
	instListLength = 0;

	scopeDepth = 0;

	symTable = std::vector<std::unordered_map<std::string, SSAValue*>>();
	symTableCopy = std::vector<std::unordered_map<std::string, SSAValue*>>();

	inOrder = std::vector<std::vector<SSAValue*>>();

}



void SSA::generateLiveRanges(BasicBlock* bb, std::set<SSAValue*>& liveRanges, std::vector<SSAValue*>& phis, SSAValue* instTail, SSAValue* stopAt) {

	SSAValue* iter = instTail;
	bool nopEncountered = false;
	int nopEncounteredCount = 0;
	while (iter != stopAt && iter->op != CONST) {
		
		for (SSAValue* val : liveRanges) {
			std::cout << val->id << ", ";
		}
		std::cout << std::endl;
		if (iter->op == PHI) {
			phis.push_back(iter);
		}
		if (iter->op == NOP) {
			nopEncountered = true;
			nopEncounteredCount += 1;
		}
		if (nopEncountered && bb->joinType == "while" && bb->numVisits == 1) {
			// above the phi instructions in while join block
			break;
		} else if (nopEncountered && bb->joinType == "while" && bb->numVisits == 2 && nopEncounteredCount == 1) {
			for (SSAValue* phi : phis) {
				liveRanges.insert(phi->operand1);
			}
			phis = std::vector<SSAValue*>();
			nopEncountered = false;

		}
		bool deadCode = false;
		if (liveRanges.find(iter) != liveRanges.end()) {
			liveRanges.erase(iter);
		} else {
			if ( (iter->op >= BRA && iter->op <= BGT) || 
				 (iter->op == write) || (iter->op == writeNL)) {

			} else {
				deadCode = true;
			}
		}
		if ((iter->op < BRA || iter->op > NOP) && !iter->isVoidCall) {
			if (deadCode) {
				iter->deadCode = deadCode;
			}

			std::set<SSAValue*> iGraphSet;
			try {
				iGraphSet = iGraph.at(iter);
			} catch (std::out_of_range& oor) {
				iGraphSet = std::set<SSAValue*>();
			}

			for (SSAValue* lrVal : liveRanges) {
				iGraphSet.insert(lrVal);

			}
			if (!iter->deadCode) {
				iGraph.insert({ iter, iGraphSet });
			}

		}
		if (iter->op == call) {
			for (SSAValue* callArg : iter->callArgs) {
				if (callArg->op != CONST) {
					liveRanges.insert(callArg);
				}
			}
		} else if (iter->op == PHI) {
			// do nothing
		} else {
			if (iter->operand1 != nullptr && iter->operand1->op != CONST) {
				if (iter->operand1->op != NOP) {
					liveRanges.insert(iter->operand1);

				}
			}
			if (iter->operand2 != nullptr && iter->operand2->op != CONST) {
				if (iter->operand2->op != NOP) {
					liveRanges.insert(iter->operand2);

				}
			}

		}
		iter = iter->prev;
	}
}

void SSA::printLiveRanges() {
	std::cout << "digraph iGraph {" << std::endl;
	std::set<SSAValue*> labelsCreated;
	std::vector<std::tuple<int, int>> madeEdges;
	for (auto kv : iGraph) {
		std::string id = std::to_string(kv.first->id);
		std::string output;
		if (!kv.first->deadCode) {
			if (labelsCreated.find(kv.first) == labelsCreated.end()) {
				output = id + "[label=\"" + kv.first->instCFGRepr() + "\"]\n";
			}
			labelsCreated.insert(kv.first);
		} else {
			output = id + "[style=filled fillcolor=\"red\" label=\"" + kv.first->instCFGRepr() + " - DEAD\"]\n";
		}
		
		for (SSAValue* interElem : kv.second) {
			bool addEdge = true;
			for (auto tup : madeEdges) {
				if ( (std::get<0>(tup) == kv.first->id && std::get<1>(tup) == interElem->id)
					or (std::get<0>(tup) == interElem->id && std::get<0>(tup) == kv.first->id)) {
					addEdge = false;
				}
			}
			if (addEdge) {
				if (labelsCreated.find(interElem) == labelsCreated.end()) {
					output += std::to_string(interElem->id) + "[label=\"" + interElem->instCFGRepr() + "\"]\n";
					labelsCreated.insert(interElem);
				}
				output += id + " -> " + std::to_string(interElem->id) + " [arrowhead=none]\n";
				std::tuple<int, int> newTup1 = std::make_tuple(kv.first->id, interElem->id);
				std::tuple<int, int> newTup2 = std::make_tuple(interElem->id, kv.first->id);
				madeEdges.push_back(newTup1);
				madeEdges.push_back(newTup2);

			}
		}
		std::cout << output << std::endl;
	}
	std::cout << "}" << std::endl;
}

bool elemExistsInStack(std::stack<BasicBlock*> stackCopy, BasicBlock* toFind) {
	if (stackCopy.top() != toFind) {
		if (stackCopy.size() == 1) {
			return false;
		} else {
			stackCopy.pop();
			return elemExistsInStack(stackCopy, toFind);
		}
	}
}


void SSA::traverseBasicBlocks(BasicBlock* startBlock) {
	std::queue<BasicBlock*> q;
	q.push(startBlock);
	while (q.size() != 0) {

		BasicBlock* currentBlock = q.front();
		q.pop();
		if (!currentBlock->visited) {
			std::cout << currentBlock->getID() << std::endl;
			handleTraverseStep(currentBlock);
		}
		if (! (currentBlock->joinType == "while") && !(currentBlock->splitType == "if") ) {
			currentBlock->visited = true;
		} 
		startBlock->visited = true;
		if (currentBlock->joinType == "while") {
			if (currentBlock->numVisits == 1) {

				q.push(currentBlock->loopBackFrom);
				currentBlock->numVisits = 1;
				currentBlock->loopBackFrom->numVisits += 1;
			} else if (currentBlock->numVisits == 2) {
				BasicBlock* ftBlock = currentBlock->fallThroughFrom;
				BasicBlock* brBlock = currentBlock->branchFrom;
				if (ftBlock != nullptr && ftBlock->id != -1) {
					q.push(ftBlock);
					ftBlock->numVisits += 1;
				} if (brBlock != nullptr && brBlock->id != -1) {
					q.push(brBlock);
					brBlock->numVisits += 1;
				}
				currentBlock->numVisits += 1;
				currentBlock->visited = true;

			}
		} else if (currentBlock->splitType == "if") {
			if (currentBlock->numVisits == 2) {
				BasicBlock* ftBlock = currentBlock->fallThroughFrom;
				BasicBlock* brBlock = currentBlock->branchFrom;
				if (ftBlock != nullptr && ftBlock->id != -1) {
					std::cout << "In if split" << std::endl;
					std::cout << "Adding ftfrom block " << ftBlock->id << " from the " << currentBlock->id << " block" << std::endl;
					q.push(ftBlock);
					ftBlock->numVisits += 1;
				} if (brBlock != nullptr && brBlock->id != -1) {
					q.push(brBlock);
					brBlock->numVisits += 1;
				}
				currentBlock->numVisits += 1;
				currentBlock->visited = true;


			} else {
				std::cout << "split type is if but num visits is NOT 2 fuckers!!" << std::endl;
			}

		} else {
			BasicBlock* ftBlock = currentBlock->fallThroughFrom;
			BasicBlock* brBlock = currentBlock->branchFrom;


			if (ftBlock != nullptr && ftBlock->joinType == "while" && ftBlock->numVisits == 1) {
				q.push(ftBlock);
				ftBlock->numVisits += 1;
			} else if (ftBlock != nullptr && !ftBlock->visited && ftBlock->id != -1) {

				q.push(ftBlock);
				ftBlock->numVisits += 1;
			}
			if (brBlock != nullptr && !brBlock->visited && brBlock->id != -1) {
				q.push(brBlock);
				brBlock->numVisits += 1;

			}
		}
	}

}

void SSA::correctBasicBlockIssues() {
	for (BasicBlock* bb : basicBlocks) {
		if (bb->tail != nullptr && bb->tail->op == BRA) {
			BasicBlock* nextBlock = bb->fallThrough;

			if (nextBlock != nullptr) {
				bb->fallThrough = nullptr;
				bb->branch = nextBlock;
				nextBlock->branchFrom = bb;
			}
		}
		if (bb->fallThrough != nullptr && bb->fallThrough->id != bb->id + 1) {
			BasicBlock* correctBlock = findBBWithID(bb->id + 1);
			bb->fallThrough = correctBlock;
			correctBlock->fallThroughFrom = bb;
		}
	}
}

BasicBlock* SSA::findBBWithID(int id) {
	for (BasicBlock* bb : basicBlocks) {
		if (bb->id == id) {
			return bb;
		}
	}
	return nullptr;
}

BasicBlock* SSA::getBBListHead() {
	return bbListHead;
}

BasicBlock* SSA::getBBTail() {
	return context;
}

void BasicBlock::unionLiveRanges(BasicBlock* toUnionFrom, std::string sideOfPhi) {
	if (toUnionFrom != nullptr) {
		for (SSAValue* val : toUnionFrom->liveRanges) {
			std::cout << val->id << std::endl;
			liveRanges.insert(val);
		}
		for (SSAValue* phi : toUnionFrom->phis) {
			if (sideOfPhi == "left") {
				if (phi->operand1->op != CONST) {
					liveRanges.insert(phi->operand1);
				}
			} else if (sideOfPhi == "right") {
				if (phi->operand2->op != CONST) {
					liveRanges.insert(phi->operand2);
				}
			} else {
				// blank for now
			}
		}
	}
}

void SSA::handleTraverseStep(BasicBlock* bb) {

	if (bb->conditionalBlockType == "ifThenElse-Else") {
		if (bb->branch == nullptr && bb->loop == nullptr) {
			// at the end of else in if-then-else struct
			bb->unionLiveRanges(bb->fallThrough, "right");
		} else {
			std::cout << "inside of this condition" << std::endl;
			bb->unionLiveRanges(bb->fallThrough, "left");
			bb->unionLiveRanges(bb->branch, "right");
		}
	} else if (bb->conditionalBlockType == "ifThenElse-Then") {
		if (bb->fallThrough == nullptr && bb->loop == nullptr) {
			// at the end of then in if-then-else struct
			bb->unionLiveRanges(bb->branch, "left");
		} else {
			bb->unionLiveRanges(bb->fallThrough, "left");
			bb->unionLiveRanges(bb->branch, "right");
		}

	} else if (bb->conditionalBlockType == "ifThen-Then") {
		if (bb->branch == nullptr && bb->loop == nullptr) {
			std::cout << "in this case" << std::endl;
			// at the end of then in if-then struct
			bb->unionLiveRanges(bb->fallThrough, "left");
		} else {
			bb->unionLiveRanges(bb->fallThrough, "");
			bb->unionLiveRanges(bb->branch, "right");
		}
	} else {
		if (bb->loop != nullptr) {
			// end of while body struct
			bb->unionLiveRanges(bb->loop, "right");
		} else {
			// need to handle cond with only then block here
			if (bb->splitType == "if") {
				if (bb->branch->joinType == "while") {
					if (bb->fallThrough->conditionalBlockType == "ifThen-Then") {

						bb->unionLiveRanges(bb->branch, "right");

					} else {
						bb->unionLiveRanges(bb->branch, "left");

					}
				} else {
					bb->unionLiveRanges(bb->branch, "right");

				}
			} else {
				bb->unionLiveRanges(bb->branch, "");

			}

			bb->unionLiveRanges(bb->fallThrough, "");
			bb->unionLiveRanges(bb->loop, "");

		}
	}


	SSAValue* stopAt; 
	if (bb->head != nullptr) {
		stopAt = bb->head->prev;
	} else {
		stopAt = nullptr;
	}
	generateLiveRanges(bb, bb->liveRanges, bb->phis, bb->tail, stopAt);

}

IGraphNode* SSA::findInIGraphNodes(SSAValue* toFind) {
	for (IGraphNode* node : iGraphNodes) {
		if (node->initValue == toFind) {
			return node;
		}
	}
	return nullptr;
}

IGraphNode::IGraphNode(SSAValue* val, int nodeID) {
	initValue = val;
	values.insert(val);
	id = nodeID;
	moveTarget = nullptr;
	visited = false;
	color = -1;
}

void SSA::generateIGraphNodes() {
	for (auto kv : iGraph) {
		IGraphNode* node = findInIGraphNodes(kv.first);
		if (node == nullptr) {
			node = new IGraphNode(kv.first, kv.first->id);
			iGraphNodes.push_back(node);

			for (SSAValue* val : kv.second) {
				IGraphNode* connectToNode = findInIGraphNodes(val);
				if (connectToNode == nullptr) {
					connectToNode = new IGraphNode(val, val->id);
					iGraphNodes.push_back(connectToNode);
				}
			}
		}
	}
	for (auto kv : iGraph) {
		IGraphNode* node = findInIGraphNodes(kv.first);
		std::vector<IGraphNode*> toErase;
		for (SSAValue* connectedToVal : kv.second) {
			IGraphNode* connectToNode = findInIGraphNodes(connectedToVal);

			bool erased = false;
			if (node->initValue->deadCode) {
				toErase.push_back(node);
				erased = true;
			}
			if (connectToNode != nullptr && connectToNode->initValue->deadCode) {
				toErase.push_back(connectToNode);
				erased = true;
			}
			if (!erased) {
				if (connectToNode != nullptr) {
					node->connectedTo.insert(connectToNode);
					connectToNode->connectedTo.insert(node);

				}
			}

		}
		for (auto needToErase : toErase) {
			int index = findIndexOfIGraphNode(needToErase);
			if (index >= 0) {
				iGraphNodes.erase(iGraphNodes.begin() + index);

			}
		}

	}
}

bool SSA::checkInterferesWith(IGraphNode* node, SSAValue* instr) {
	for (IGraphNode* iNode : node->connectedTo) {
		for (SSAValue* val : iNode->values) {
			if (val == instr) {
				return true;
			}
		}
	}
	return false;
}

int SSA::findIndexOfIGraphNode(IGraphNode* toFindNode) {

	for (int i = 0; i < iGraphNodes.size(); i++) {
		if (iGraphNodes.at(i) == toFindNode) {
			return i;
		}
	}
	return -1;
}

void SSA::addNewEdges(IGraphNode* newConnection, IGraphNode* oldConnection) {
	std::vector<IGraphNode*> toEraseFrom;
	for (IGraphNode* node : iGraphNodes) {
		for (IGraphNode* connection : node->connectedTo) {

			if (connection == oldConnection) {
				node->connectedTo.insert(newConnection);
				toEraseFrom.push_back(node);
			}
		}
	}
	for (IGraphNode* node : toEraseFrom) {
		node->connectedTo.erase(oldConnection);
	}
}
void SSA::coalescePhi(IGraphNode* node, std::vector<IGraphNode*>& toDelete) {
	node->visited = true;
	SSAValue* val = node->initValue;
	SSAValue* phiOperand1 = val->operand1;
	SSAValue* phiOperand2 = val->operand2;


	if (phiOperand1 != nullptr && phiOperand1->op != CONST) {

		if (!checkInterferesWith(node, phiOperand1)) {
			IGraphNode* phiNode = nullptr;
			if (phiOperand1->op == PHI) {
				phiNode = findInIGraphNodes(phiOperand1);
				if (phiNode == nullptr) {
				} else {
					coalescePhi(phiNode, toDelete);
					for (SSAValue* nodeVal : phiNode->values) {
						node->values.insert(nodeVal);
					}

				}
			} else {
				// operand1 is not a phi instr
				phiNode = findInIGraphNodes(phiOperand1);
				for (SSAValue* nodeVal : phiNode->values) {
					node->values.insert(nodeVal);
				}
			}
			node->values.insert(phiOperand1);

			if (phiNode != nullptr) {
				for (IGraphNode* newInter : phiNode->connectedTo) {
					node->connectedTo.insert(newInter);
				}
				toDelete.push_back(phiNode);
				addNewEdges(node, phiNode);
				node->values.erase(val);
			}
		} else {
			IGraphNode* phiOp1Node = findInIGraphNodes(phiOperand1);
			phiOp1Node->moveTarget = node;
		}
	}
	if (phiOperand2 != nullptr && phiOperand2->op != CONST) {

		if (!checkInterferesWith(node, phiOperand2)) {
			IGraphNode* phiNode = nullptr;
			if (phiOperand2->op == PHI) {
				phiNode = findInIGraphNodes(phiOperand2);
				coalescePhi(phiNode, toDelete);
				if (phiNode == nullptr) {
				}
				for (SSAValue* nodeVal : phiNode->values) {
					node->values.insert(nodeVal);
				}
			} else {
				// operand2 of phi is not a phi
				phiNode = findInIGraphNodes(phiOperand2);
				for (SSAValue* nodeVal : phiNode->values) {
					node->values.insert(nodeVal);
				}
			}
			node->values.insert(phiOperand2);
			if (phiNode != nullptr) {
				for (IGraphNode* newInter : phiNode->connectedTo) {
					node->connectedTo.insert(newInter);
				}
				toDelete.push_back(phiNode);
				addNewEdges(node, phiNode);
				node->values.erase(val);
			}
		} else {
			IGraphNode* phiOp2Node = findInIGraphNodes(phiOperand2);
			phiOp2Node->moveTarget = node;
		}
	}
}

void SSA::clusterIGraphNodes() {

	std::vector<IGraphNode*> toDelete;
	for (IGraphNode* node : iGraphNodes) {
		SSAValue* val = node->initValue;
		if (val->op == PHI && !node->visited) {
			coalescePhi(node, toDelete);
		}
	}
	for (auto node : toDelete) {
		int index = findIndexOfIGraphNode(node);
		if (index >= 0) {
			iGraphNodes.erase(iGraphNodes.begin() + index);
		}
	}

}

std::string IGraphNode::iGraphNodeRepr() {
	std::string output = std::to_string(id) + "[shape=record, style=filled,colorscheme=pastel19,fillcolor=" + std::to_string(color) + ",label=\"< b > " + std::to_string(id) + "| {Init Value : " + initValue->instCFGRepr() + " | ";
	output += "Values |";
	for (SSAValue* val : values) {
		output += val->instCFGRepr() + "|";
	}
	output += "Connected to |";
	for (IGraphNode* node : connectedTo) {
		output += " " + std::to_string(node->id) + ",";
	}
	output.pop_back();
	if (moveTarget != nullptr) {
		output += "| Move Target |" + std::to_string(moveTarget->id);
	}
	output += "| Color | " + std::to_string(color);
	output += "}\"]\n";
	for (IGraphNode* node : connectedTo) {
		output += std::to_string(id) + "--" + std::to_string(node->id) + "[arrowhead=none]\n";
	}
	return output;
}

void SSA::printClusteredIGraph() {
	std::cout << "strict graph clusteredIGraph {" << std::endl;
	for (IGraphNode* node : iGraphNodes) {
		if (node != nullptr) {
			std::cout << node->iGraphNodeRepr() << std::endl;

		}
	}
	std::cout << "}" << std::endl;
}

IGraphNode* SSA::pickNode(std::vector<IGraphNode*> nodes) {
	int numRegisters = 5;
	IGraphNode* minNode = nullptr;
	int minNeighbors = 100;
	for (auto node : nodes) {
		int numNeighbors = node->connectedTo.size();
		if (numNeighbors < minNeighbors) {
			minNode = node;
			minNeighbors = numNeighbors;
		}
		if (numNeighbors < numRegisters) {
			return node;
		}
	}
	return minNode;
}

int SSA::pickColor(IGraphNode* node) {
	int possibleColors[5] = { 1, 2, 3, 4, 5 };
	std::vector<int> neighborColors;
	for (IGraphNode* connection : node->connectedTo) {
		if (connection != nullptr && connection->color != -1) {
			for (int i = 0; i < 5; i++) {
				neighborColors.push_back(connection->color);
				if (possibleColors[i] == connection->color) {
					possibleColors[i] = 0;
				}
			}
		}
	}
	bool colorExists = false;
	for (int color : possibleColors) {
		if (color != 0) {
			colorExists = true;
			return color;
		}
	}
	if (!colorExists) {
		if (virtualRegColors.size() == 0) {
			virtualRegColors.push_back(6);
		}
		bool foundSuitableVRegColor = false;
		for (int vRegColor : virtualRegColors) {
			bool noNeighborSharesColor = true;
			for (int neighborColor : neighborColors) {
				if (vRegColor == neighborColor) {
					noNeighborSharesColor = false;
				}
			}
			if (noNeighborSharesColor) {
				foundSuitableVRegColor = true;
				return vRegColor;
				break;
			}
		}
		if (!foundSuitableVRegColor) {
			int newRegColor = virtualRegColors.back() + 1;
			virtualRegColors.push_back(newRegColor);
			return newRegColor;
		}
	}
}

void SSA::colorGraph() {

	int numRegisters = 5;
	IGraphNode* node = pickNode(iGraphNodes);
	if (node == nullptr) {
		return;
	}
	int index = findIndexOfIGraphNode(node);
	if (iGraphNodes.size() != 0 && index >= 0) {
		iGraphNodes.erase(iGraphNodes.begin() + index);
	}
	std::vector<IGraphNode*> addBackTo;
	for (auto editNode : node->connectedTo) {
		if (editNode != nullptr) {
			addBackTo.push_back(editNode);
			editNode->connectedTo.erase(node);
		}
	}
	if (iGraphNodes.size() != 0) {

		colorGraph();
	} else {
		std::cout << "size is 0" << std::endl;
	}
	iGraphNodes.push_back(node);

	for (auto editNode : addBackTo) {
		if (editNode != nullptr) {
			editNode->connectedTo.insert(node);
		}
	}
	node->color = pickColor(node);

	return;


}

void SSA::generateRegisters() {
	for (auto node : iGraphNodes) {
		try {
			Register* reg = registers.at(node->color);
			for (auto val : node->values) {
				reg->values.insert(val);
			}
			reg->values.insert(node->initValue);

		}
		catch (std::out_of_range& oor) {
			Register* newReg = new Register();
			newReg->id = node->color;
			if (node->color > 5) {
				newReg->isVirtual = true;
			}
			for (auto val : node->values) {
				newReg->values.insert(val);
			}
			newReg->values.insert(node->initValue);
			registers.insert({ node->color, newReg });
		}
	}
}

void SSA::printRegisters() {
	std::cout << "digraph regs {" << std::endl;
	for (auto kv : registers) {
		Register* reg = kv.second;
		std::string outString;
		outString = std::to_string(reg->id) + "[shape = record, style = filled, colorscheme = pastel19, fillcolor =" + std::to_string(reg->id) + ", label = \"<b>" + std::to_string(reg->id) + "|{";
		for (auto val : reg->values) {
			outString += val->instCFGRepr() + "|";
		}
		outString.pop_back();
		outString += "}\"];";
		std::cout << outString << std::endl;
	}
	std::cout << "}" << std::endl;
}

void SSA::setRegisters() {
	for (auto kv : registers) {
		for (SSAValue* val : kv.second->values) {
			val->regNum = kv.first;
		}
	}
	for (auto node : iGraphNodes) {
		IGraphNode* moveTarget = node->moveTarget;
		if (moveTarget != nullptr) {
			for (SSAValue* val : node->values) {
				val->regToMoveTo = moveTarget->initValue->regNum;
			}
		}
	}
}

void SSA::cleanInstList() {
	int labelCount = 0;
	SSAValue* iter = instList;

	while (iter != nullptr) {

		if (iter->deadCode) {
			SSAValue* iterPrev = iter->prev;
			SSAValue* iterNext = iter->next;
			BasicBlock* containingBB = iter->containingBB;
			if (iter == containingBB->head) {
				if (iterNext != nullptr) {
					if (iterNext->containingBB == containingBB) {
						containingBB->head = iterNext;
					} else {
						containingBB->head = nullptr;
					}
				} else {
					containingBB->head = nullptr;
					containingBB->tail = nullptr;
				}
			}
			if (iter == containingBB->tail) {
				if (iterNext != nullptr) {
					if (iterNext->containingBB == containingBB) {
						containingBB->tail = iterNext;
					} else {
						if (iterPrev->containingBB == containingBB) {
							containingBB->tail = iterPrev;
						} else {
							containingBB->tail = nullptr;
							containingBB->head = nullptr;
						}
					}
				} else {
					if (iterPrev->containingBB == containingBB) {
						containingBB->tail = iterPrev;
					} else {
						containingBB->tail = nullptr;
						containingBB->head = nullptr;
					}
				}
			}
			if (iterNext != nullptr) {
				iterPrev->next = iterNext;
				iterNext->prev = iterPrev;
			} else {
				iterPrev->next = nullptr;
			}


		}

		else if (iter->op == PHI) {
			SSAValue* phiOperand1 = iter->operand1;
			SSAValue* phiOperand2 = iter->operand2;
			if (phiOperand1->regToMoveTo != 0 || phiOperand1->op == CONST) {
				SSAValue* moveInstr;
				BasicBlock* bbToAddMoveIn;
				if (phiOperand1->op == CONST) {
					if (iter->containingBB->branchFrom != nullptr && iter->containingBB->branchFrom->conditionalBlockType == "ifThenElse-Then") {
						bbToAddMoveIn = iter->containingBB->branchFrom;
					} else if (iter->containingBB->fallThroughFrom != nullptr && iter->containingBB->fallThroughFrom->conditionalBlockType == "ifThen-Then") {
						bbToAddMoveIn = iter->containingBB->fallThroughFrom;
					} else {
						bbToAddMoveIn = iter->containingBB;
					}
					moveInstr = SSACreateConstMove(phiOperand1, iter->regNum);
				} else {
					bbToAddMoveIn = phiOperand1->containingBB;
					moveInstr = SSACreateMove(phiOperand1->regNum, phiOperand1->regToMoveTo);

				}
				if (bbToAddMoveIn != iter->containingBB) {
					if (bbToAddMoveIn->tail != nullptr) {
						SSAValue* currentTail = bbToAddMoveIn->tail;
						SSAValue* currentTailNext = currentTail->next;

						if (currentTail->op == BRA) {
							SSAValue* currentTailPrev = currentTail->prev;
							currentTailPrev->next = moveInstr;
							moveInstr->prev = currentTailPrev;
							moveInstr->next = currentTail;
							currentTail->prev = moveInstr;
							if (currentTail == bbToAddMoveIn->head) {
								bbToAddMoveIn->head = moveInstr;
							}
						} else {
							currentTail->next = moveInstr;
							moveInstr->prev = currentTail;
							moveInstr->next = currentTailNext;
							currentTailNext->prev = moveInstr;
							bbToAddMoveIn->tail = moveInstr;

						}


					} else {
						
						bbToAddMoveIn->head = moveInstr;
						bbToAddMoveIn->tail = moveInstr;
						moveInstr->prev = bbToAddMoveIn->fallThroughFrom->tail;
						bbToAddMoveIn->fallThroughFrom->tail->next = moveInstr;
						moveInstr->next = bbToAddMoveIn->fallThrough->head;
						bbToAddMoveIn->fallThrough->head->prev = moveInstr;
					}

				} else {
					SSAValue* whileJoinIter = bbToAddMoveIn->tail;
					while (whileJoinIter != bbToAddMoveIn->head->prev) {
						// need to check when nop is first instruction of BB
						if (whileJoinIter->op == NOP) {
							SSAValue* nopPrev = whileJoinIter->prev;
							nopPrev->next = moveInstr;
							moveInstr->prev = nopPrev;

							moveInstr->next = whileJoinIter;
							whileJoinIter->prev = moveInstr;
							break;
						}
						whileJoinIter = whileJoinIter->prev;
					}
					if (whileJoinIter == bbToAddMoveIn->head) {
						bbToAddMoveIn->head = moveInstr;
					}

				}

			}
			if (phiOperand2->regToMoveTo != 0 || phiOperand2->op == CONST) {

				SSAValue* moveInstr;
				BasicBlock* bbToAddMoveIn;
				if (phiOperand2->op == CONST) {
					if (iter->containingBB->fallThroughFrom != nullptr && iter->containingBB->fallThroughFrom->conditionalBlockType == "ifThenElse-Else") {
						bbToAddMoveIn = iter->containingBB->fallThroughFrom;
					} else if (iter->containingBB->branchFrom != nullptr && iter->containingBB->branchFrom->splitType == "if") {
						bbToAddMoveIn = iter->containingBB->branchFrom;
					} else {
						bbToAddMoveIn = iter->containingBB;
					}
					moveInstr = SSACreateConstMove(phiOperand2, iter->regNum);
				} else {
					bbToAddMoveIn = phiOperand2->containingBB;
					moveInstr = SSACreateMove(phiOperand2->regNum, phiOperand2->regToMoveTo);

				}
				SSAValue* currentTail = bbToAddMoveIn->tail;
				SSAValue* currentTailNext = currentTail->next;

				if (currentTail->op == BRA) {
					SSAValue* currentTailPrev = currentTail->prev;
					currentTailPrev->next = moveInstr;
					moveInstr->prev = currentTailPrev;
					moveInstr->next = currentTail;
					currentTail->prev = moveInstr;
				} else {
					currentTail->next = moveInstr;
					moveInstr->prev = currentTail;
					moveInstr->next = currentTailNext;
					currentTailNext->prev = moveInstr;

				}
				if (bbToAddMoveIn->splitType == "if") {
					BasicBlock* moveBB = new BasicBlock();
					moveBB->id = maxBlockID++;
					moveBB->funcName = funcName;
					basicBlocks.push_back(moveBB);

					BasicBlock* joinBlock = bbToAddMoveIn->branch;
					if (moveInstr->prev->op >= BNE && moveInstr->prev->op <= BGT) {
						SSAValue* finalTarget = moveInstr->prev->operand2;
						SSAValue* newNop = SSACreateNop();
						newNop->id = maxID++;
						moveInstr->prev->operand2 = newNop;
						newNop->label = funcName + std::to_string(labelCount);

						moveBB->head = newNop;
						SSAValue* braInstr = new SSAValue();
						braInstr->op = BRA;
						braInstr->id = maxID++;
						braInstr->operand1 = finalTarget;
						moveBB->tail = braInstr;
						
						SSAValue* moveInstrPrev = moveInstr->prev;
						SSAValue* moveInstrNext = moveInstr->next;
						moveInstrPrev->next = newNop;
						newNop->prev = moveInstrPrev;

						newNop->next = moveInstr;
						moveInstr->prev = newNop;
						

						moveInstr->next = braInstr;
						braInstr->prev = moveInstr;

						braInstr->next = moveInstrNext;
						moveInstrNext->prev = braInstr;
						

						bbToAddMoveIn->branch = moveBB;
						moveBB->branchFrom = bbToAddMoveIn;
						moveBB->branch = joinBlock;
						joinBlock->branchFrom = moveBB;
						

					} else {
						// bra instr
					}
				} else {
					bbToAddMoveIn->tail = moveInstr;
				}
			}
			

			SSAValue* iterPrev = iter->prev;
			SSAValue* iterNext = iter->next;
			BasicBlock* containingBB = iter->containingBB;
			if (iter == containingBB->head) {
				if (iterNext != nullptr) {
					if (iterNext->containingBB == containingBB) {
						containingBB->head = iterNext;
					} else {
						containingBB->head = nullptr;
					}
				} else {
					containingBB->head = nullptr;
					containingBB->tail = nullptr;
				}
			}
			

			if (iter == containingBB->tail) {
				if (iterNext != nullptr) {
					if (iterNext->containingBB == containingBB) {
						containingBB->tail = iterNext;
					} else {
						if (iterPrev->containingBB == containingBB) {
							containingBB->tail = iterPrev;
						} else {
							containingBB->tail = nullptr;
							containingBB->head = nullptr;
						}
					}
				} else {
					if (iterPrev->containingBB == containingBB) {
						containingBB->tail = iterPrev;
					} else {
						containingBB->tail = nullptr;
						containingBB->head = nullptr;
					}
				}
			}
			

			if (iterNext != nullptr) {
				iterPrev->next = iterNext;
				iterNext->prev = iterPrev;
			} else {
				iterPrev->next = nullptr;
			}
		}

		if (iter != nullptr && iter->op == BRA) {
			iter->operand1->label = funcName + std::to_string(labelCount);
			labelCount++;
		}
		if (iter != nullptr && iter->op >= BNE && iter->op <= BGT) {
			iter->operand2->label = funcName + std::to_string(labelCount);
			labelCount++;
		}
		if (iter->op == NOP) {
			if (iter->containingBB->loop != nullptr) {
				// nop at top of while body
				
				SSAValue* nopPrevInst = iter->prev;
				SSAValue* nopNextInst = iter->next;
				nopPrevInst->next = nopNextInst;
				nopNextInst->prev = nopPrevInst;
				if (iter->containingBB == nopNextInst->containingBB) {
					iter->containingBB->head = nopNextInst;
				} else {
					iter->containingBB->head = nullptr;
					iter->containingBB->tail = nullptr;
				}
			}
			
		}

		iter = iter->next;
	}

}
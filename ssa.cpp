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
	if (operand->op == NOP) {
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
/*	if (deadCode) {
		output = "";
	} else */
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
	//std::cout << "In update nop" << std::endl;
	nopInst->id = maxID++;
	addSSAValue(nopInst);
	addInOrder(nopInst);

}	

void SSA::addInOrder(SSAValue* newInst) {
	inOrder.back().push_back(newInst);
}

SSAValue* SSA::findPrevDomWithOpcode(opcode operation) {
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
	//for (auto kv : lastScope) {
	//	SSAValue* prevOccur = findSymbol(kv.first);	
	//	phiMap.insert({ prevOccur, kv.second });
	//}
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
		//current->instRepr();
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
	//std::cout << head->id << std::endl;
	//if (head == nullptr) {
	//	return "";
	//} else {
	//	std::cout << head->id << std::endl;
	//	return head->bbRepr();
	//}
	//std::string ft = reprBasicBlocks(head->fallThrough);
	//std::string br = reprBasicBlocks(head->branch);
	////std::cout << ft << "\n" << br << std::endl;
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


//std::string SSA::genBBStart(int bbID) {
//	std::string bbIDStr = std::to_string(bbID);
//	std::string currentBB = "bb" + bbIDStr + "[shape=record, label=\" < b > BB" + bbIDStr + " | {";
//	return currentBB;
//}

//std::tuple<std::vector<BasicBlock*>, std::vector<BBEdge*>> SSA::genBasicBlocks() {
//	/*
//		if a branch instruction is found
//			close current bb (include branch inst in current bb)
//			create new bb
//			store branch-to id for new block
//		else
//		    if id was previously a branch-to id
//			    close current bb
//				create new bb
//			else
//			    just add inst to bb
//	*/
//
//	SSAValue* current = instList; 
//	SSAValue* prevCurrent = current;
//	std::unordered_map<int, BasicBlock*> branchToBlocks; // inst ids for when to create new bb
//	std::vector<BasicBlock*> basicBlocks;
//	std::vector<BBEdge*> bbEdges;
//
//	BasicBlock* currentBB = nullptr;
//	int bbID = 1;
//
//
//	while (current != nullptr) {
//		if (currentBB == nullptr) {
//			currentBB = new BasicBlock(true);
//			currentBB->setBBID(1);
//			currentBB->setHead(current);
//			currentBB->setTail(current);
//		}
//		if (current->op <= 17 && current->op >= 11) {
//			// any branch inst
//			// close current block, create new block, and store branch-to id for new block later
//			currentBB->setTail(current);
//			bbID += 1;
//			basicBlocks.push_back(currentBB);
//
//
//
//
//
//			currentBB = new BasicBlock(true);
//			currentBB->setHead(current->next);
//			currentBB->setTail(current);
//			currentBB->setBBID(bbID);
//
//			BBEdge* newEdge = new BBEdge(basicBlocks.back(), currentBB, "ft");
//			bbEdges.push_back(newEdge);
//
//			
//
//
//
//			// at this point, currentBB is FT block (edge to it from split block must be ft edge)
//
//			int newBlockID;
//			if (current->op == BRA) {
//				// bra inst
//				newBlockID = current->operand1->id;
//
//			} else {
//				// all other branch insts (12 <= op <= 17)
//				newBlockID = current->operand2->id;
//
//			}
//			// before creating branch block, first check that the inst does not exist above
//			bool found = false;
//			BasicBlock* jumpToBlock = nullptr;
//			for (BasicBlock* bb : basicBlocks) {
//				SSAValue* bbHead = bb->getHead();
//				SSAValue* bbTail = bb->getTail();
//				while (bbHead != bbTail->next) {
//					if (bbHead->id == newBlockID) {
//						found = true;
//						jumpToBlock = bb;
//						break;
//					}
//
//					bbHead = bbHead->next;
//				}
//			}
//			std::cout << "FOUND is " << found << std::endl;
//			if (!found) {
//				BasicBlock* branchBlock = new BasicBlock(false);
//				branchToBlocks.insert({ newBlockID, branchBlock });
//				BBEdge* newEdge = new BBEdge(basicBlocks.back(), branchBlock, "br");
//				bbEdges.push_back(newEdge);
//			} else {
//				BBEdge* newEdge = new BBEdge(basicBlocks.back(), jumpToBlock, "br");
//				bbEdges.push_back(newEdge);
//
//			}
//
//		} else {
//			bool mustCreateNewBlock = false;
//			// check if current inst id needs to be start of new bb
//			BasicBlock* branchToBlock = nullptr;
//			for (auto kv : branchToBlocks) {
//				if (current->id == kv.first) {
//					branchToBlock = kv.second;
//					break;
//				}
//			}
//			if (branchToBlock != nullptr) {
//				basicBlocks.push_back(currentBB);
//				bbID += 1;
//				currentBB = branchToBlock;
//				currentBB->setBBID(bbID);
//				currentBB->setHead(current);
//				currentBB->setTail(current);
//				// at this point, new block is a branch-to block, must have edge type of branch
//			} else {
//				// just add instruction to current block
//				if (currentBB->getHead() == nullptr) {
//					currentBB->setHead(current);
//				}
//				currentBB->setTail(current);
//			}
//		}
//		prevCurrent = current;
//		current = current->next;
//	}
//	currentBB->setTail(prevCurrent); // questionable
//	basicBlocks.push_back(currentBB);
//
//	return std::make_tuple(basicBlocks, bbEdges);
	//std::tuple<std::vector<BasicBlock*>, std::vector<BBEdge*>> info;
	//info->basicBlocks = basicBlocks;
	//info->bbEdges = bbEdges;
	//return info;
//}

void SSA::gen(bool printRegs) {
	//std::tuple<std::vector<BasicBlock*>, std::vector<BBEdge*>> info = genBasicBlocks();
	//std::vector<BasicBlock*> basicBlocks = std::get<0>(info);
	//std::vector<BBEdge*> bbEdges = std::get<1>(info);

	std::cout << reprBasicBlocks(bbListHead, printRegs) << std::endl;
	//for (BasicBlock* bb: basicBlocks) {
	//	std::cout << bb->bbRepr() << std::endl;
	//}

	//for (BBEdge* edge : bbEdges) {
	//	std::cout << edge->bbEdgeRepr() << std::endl;
	//}
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
	//if (type == "while") {
	//	block->isWhile = true;
	//}
}

void SSA::initBlock(BasicBlock* blockToInit) {
	blockToInit->id = maxBlockID++;
	basicBlocks.push_back(blockToInit);

}
//void SSA::generateDotLang() {
//	
//	std::unordered_map<int, BBEdge*> branchEdgeMap;
//	std::unordered_map<int, BBEdge*> fallThroughEdgeMap;
//
//	
//	SSAValue* current = instList;
//	SSAValue* prevCurrent = nullptr;
//	std::vector<int> newBlockIDs;
//	std::vector<std::string> bbStrings;
//	std::string currentBB = "";
//	int bbID = 1;
//
//	bool isBlockNew = false;
//
//	while (current != nullptr) {
//		if (currentBB == "") {
//			currentBB = "bb1 [shape=record, label=\" < b > BB1 | {";
//		}
//
//
//
//
//		if (current->op <= 17 && current->op >= 11) {
//			// branch inst
//			// close current block, create new block, and store branch-to id for new block later
//			BBEdge* newBranchEdge = new BBEdge(bbID);
//			BBEdge* newFTEdge = new BBEdge(bbID);
//			currentBB = currentBB + current->instCFGRepr() + "}\"];";
//
//			fallThroughEdgeMap.insert({ current->id + 1, newFTEdge });
//			current->bbID = bbID;
//
//			bbStrings.push_back(currentBB);
//			bbID += 1;
//			std::string bbIDStr = std::to_string(bbID);
//			currentBB = "bb" + bbIDStr + "[shape=record, label=\" < b > BB" + bbIDStr + " | {";
//			isBlockNew = true;
//			int newBlockID;
//			if (current->op == 11) {
//				// bra inst
//				newBlockID = current->operand1->id;
//
//			} else {
//				// all other branch insts (12 <= op <= 17)
//				newBlockID = current->operand2->id;
//			}
//			
//			branchEdgeMap.insert({ newBlockID, newBranchEdge });
//			newBlockIDs.push_back(newBlockID);
//
//		} else {
//			bool mustCreateNewBlock = false;
//			for (int id : newBlockIDs) {
//				if (current->id == id) {
//					mustCreateNewBlock = true;
//					break;
//				}
//			}
//			if (mustCreateNewBlock) {
//				if (!isBlockNew) {
//					currentBB.pop_back();
//					currentBB = currentBB + "}\"];";
//					bbStrings.push_back(currentBB);
//					bbID += 1;
//					std::string bbIDStr = std::to_string(bbID);
//					currentBB = "bb" + bbIDStr + "[shape=record, label=\" < b > BB" + bbIDStr + " | {";
//
//				}
//				currentBB = currentBB + current->instCFGRepr() + "|";
//				current->bbID = bbID;
//				isBlockNew = false;
//			} else {
//				// just add instruction to current block
//				currentBB = currentBB + current->instCFGRepr() + "|";
//				current->bbID = bbID;
//
//				isBlockNew = false;
//			}
//
//		}
//		prevCurrent = current;
//		current = current->next;
//
//	}
//	currentBB.pop_back();
//	currentBB = currentBB + "}\"];";
//	bbStrings.push_back(currentBB);
//
//	current = instList;
//	while (current != nullptr) {
//
//		try {
//			BBEdge* bEdge = branchEdgeMap.at(current->id);
//			bEdge->setToBlockID(current->bbID);
//		} catch (std::out_of_range& oor) {
//			// intentionally left blank
//		}
//
//		try {
//			BBEdge* fEdge = fallThroughEdgeMap.at(current->id);
//			fEdge->setToBlockID(current->bbID);
//
//		} catch (std::out_of_range& oor) {
//			 //intentionally left blank
//		}
//		current = current->next;
//	}
//
//
//
//	std::cout << "digraph G {" << std::endl;
//	for (std::string s : bbStrings) {
//		std::cout << s << std::endl;
//	}
//
//
//
//	std::unordered_map<int, int> visited;
//	for (auto kv : branchEdgeMap) {
//		BBEdge* edge = kv.second;
//		std::cout << "bb" << edge->getFrom() << ":s -> bb" << edge->getTo() << ":n;" << std::endl;
//		visited.insert({ edge->getFrom(), edge->getTo() });
//	}
//	for (auto kv : fallThroughEdgeMap) {
//		BBEdge* edge = kv.second;
//		try {
//			int to = visited.at(edge->getFrom());
//			if (to != edge->getTo()) {
//				std::cout << "bb" << edge->getFrom() << ":s -> bb" << edge->getTo() << ":n;" << std::endl;
//
//			}
//		} catch (std::out_of_range& oor) {
//			std::cout << "bb" << edge->getFrom() << ":s -> bb" << edge->getTo() << ":n;" << std::endl;
//
//		}
//	}
//
//
//	std::cout << "}" << std::endl;
//	for (auto kv : branchEdgeMap) {
//		BBEdge* edge = kv.second;
//		std::cout << kv.first << " " << edge->getFrom() << " " << edge->getTo() << std::endl;
//	}
//}


// BasicBlock


//BasicBlock::BasicBlock(bool ft) {
//	head = nullptr;
//	tail = nullptr;
//	id = 0;
//	incomingEdgeIsFT = ft;
//}

SSAValue* BasicBlock::getHead() {
	return head;
}

//void BasicBlock::setHead(SSAValue* bbHead) {
//	head = bbHead;
//}

SSAValue* BasicBlock::getTail() {
	return tail;
}

//void BasicBlock::setTail(SSAValue* bbTail) {
//	tail = bbTail;
//}

int BasicBlock::getID() {
	return id;
}

//void BasicBlock::setBBID(int bbID) {
//	id = bbID;
//}

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

		//if (tail != nullptr && tail->op == BRA) {
		//	type = "BR";
		//}
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

	//constTable = std::unordered_map<int, SSAValue*>();
}



void SSA::generateLiveRanges(BasicBlock* bb, std::set<SSAValue*>& liveRanges, std::vector<SSAValue*>& phis, SSAValue* instTail, SSAValue* stopAt) {

	//map liverange;
	//list curLiveValues = {};
	//while curInst not head:
	//if curInst in curLiveValues :
	//curLiveValues - {curInst}
	//curLiveValues = curLiveValues + {curInst->operands};
	//liverange[curInst#] = curLiveValues
	//	curInst = curInst->prev

	SSAValue* iter = instTail;
	bool nopEncountered = false;

	while (iter != stopAt && iter->op != CONST) {
		std::cout << "Current iter is " << iter->instCFGRepr() << ", set is: " << std::endl;
		
		for (SSAValue* val : liveRanges) {
			std::cout << val->id << ", ";
		}
		std::cout << std::endl;
		if (iter->op == PHI) {
			phis.push_back(iter);
		}
		if (iter->op == NOP) {
			nopEncountered = true;
		}
		if (nopEncountered && bb->joinType == "while" && bb->numVisits == 1) {
			// above the phi instructions in while join block
			break;
		} else if (nopEncountered && bb->joinType == "while" && bb->numVisits == 2) {
			std::cout << "While Join Above the Phis Visited twice" << std::endl;
			for (SSAValue* phi : phis) {
				liveRanges.insert(phi->operand1);
			}
		}
		bool deadCode = false;
		if (liveRanges.find(iter) != liveRanges.end()) {
			std::cout << "not in dead code condition" << std::endl;
			liveRanges.erase(iter);

		} else {
			if ( (iter->op >= BRA && iter->op <= BGT) || 
				 (iter->op == write) || (iter->op == writeNL)) {

			} else {
				std::cout << "in dead code" << std::endl;
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

				//try {
				//	std::set<SSAValue*> iGraphSet = iGraph.at(iter);
				//	iGraphSet.insert(lrVal);
				//	iGraph.insert_or_assign(iter, iGraphSet);
				//}
				//catch (std::out_of_range& oor) {
				//	std::set<SSAValue*> innerSet;
				//	innerSet.insert(lrVal);
				//	iGraph.insert({ iter, innerSet });
				//	// intentionally left blank
				//}
				//try {
				//	std::set<SSAValue*> iGraphSet = iGraph.at(lrVal);
				//	//std::cout << "checking " << iter->instCFGRepr() << std::endl;
				//	iGraphSet.insert(iter);
				//}
				//catch (std::out_of_range& oor) {
				//	std::set<SSAValue*> innerSet;
				//	innerSet.insert(iter);
				//	iGraph.insert({ lrVal, innerSet });
				//
				////	// intentionally left blank
				//}

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
		//std::cout << "Items in q are: ";
		//std::queue<BasicBlock*> copy = q;
		//while (copy.size() != 0) {
		//	std::cout << copy.front()->id << "[ " << copy.front()->numVisits << " ], ";
		//	copy.pop();
		//}
		//std::cout << std::endl;
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
		//currentBlock->visited = true;
		//std::cout << "split type is " << currentBlock->splitType << std::endl;
		if (currentBlock->joinType == "while") {
			// in while join block
			//std::cout << "Current Block num visits is " << currentBlock->numVisits << std::endl;
			if (currentBlock->numVisits == 1) {
				//std::cout << "In while join" << std::endl;
				//std::cout << "Adding loopfrom block " << currentBlock->loopBackFrom->id << " from the " << currentBlock->id << " block" << std::endl;

				q.push(currentBlock->loopBackFrom);
				currentBlock->numVisits = 1;
				currentBlock->loopBackFrom->numVisits += 1;
			} else if (currentBlock->numVisits == 2) {
				BasicBlock* ftBlock = currentBlock->fallThroughFrom;
				BasicBlock* brBlock = currentBlock->branchFrom;
				if (ftBlock != nullptr && ftBlock->id != -1) {
					//std::cout << "In while join" << std::endl;
					//std::cout << "Adding ftfrom block " << ftBlock->id << " from the " << currentBlock->id << " block" << std::endl;
					q.push(ftBlock);
					ftBlock->numVisits += 1;
				} if (brBlock != nullptr && brBlock->id != -1) {
					//std::cout << "In while join" << std::endl;
					//std::cout << "Adding branchfrom block " << brBlock->id << " from the " << currentBlock->id << " block" << std::endl;
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
				//std::cout << "in if split" << std::endl;
				if (ftBlock != nullptr && ftBlock->id != -1) {
					std::cout << "In if split" << std::endl;
					std::cout << "Adding ftfrom block " << ftBlock->id << " from the " << currentBlock->id << " block" << std::endl;
					q.push(ftBlock);
					ftBlock->numVisits += 1;
				} if (brBlock != nullptr && brBlock->id != -1) {
					//std::cout << "In if split" << std::endl;
					//std::cout << "Adding branchfrom block " << brBlock->id << " from the " << currentBlock->id << " block" << std::endl;
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
				//std::cout << "Should only be adding while join here" << std::endl;
				//std::cout << "Adding ftfrom block " << ftBlock->id << " from the " << currentBlock->id << " block" << std::endl;
				q.push(ftBlock);
				ftBlock->numVisits += 1;
			} else if (ftBlock != nullptr && !ftBlock->visited && ftBlock->id != -1) {
				//std::cout << "Adding ftfrom block " << ftBlock->id << " from the " << currentBlock->id << " block" << std::endl;

				q.push(ftBlock);
				ftBlock->numVisits += 1;
			}
			if (brBlock != nullptr && !brBlock->visited && brBlock->id != -1) {
				//std::cout << "Adding branchfrom block " << brBlock->id << " from the " << currentBlock->id << " block" << std::endl;
				q.push(brBlock);
				brBlock->numVisits += 1;

			}
		}
	}
	//std::stack<BasicBlock*> s;
	//q.push(startBlock);
	//startBlock->visited = true;
	//BasicBlock* savedBlock = nullptr;
	//while (q.size() != 0) {
	//	BasicBlock* currentBlock = q.front();
	//	if (savedBlock == nullptr) {
	//		//std::cout << "saved block is empty" << std::endl;
	//	} else {
	//		//std::cout << "saved block is " << savedBlock->getID() << std::endl;
	//	}
	//	if (s.size() > 1 && savedBlock == s.top()) {
	//		//std::cout << "in savedblock == s.top() check " << std::endl;
	//		s.pop();
	//		currentBlock = savedBlock;
	//		savedBlock = s.top();
	//	} else {
	//		if (currentBlock->isWhile) {
	//			if (currentBlock->joinType == "") {
	//				// while block that has been visited already
	//				if (s.size() > 0 && currentBlock != s.top() ) {
	//					//std::cout << currentBlock->id <<" coming from here" << std::endl;
	//					//std::cout << "top of stack is " << s.top()->id << std::endl;
	//					//std::cout << "going over join block again but too early " << currentBlock->getID() << std::endl;
	//					savedBlock = currentBlock;

	//					if (q.size() > 0) {
	//						q.pop();
	//					}
	//					if (q.size() > 0) {
	//						currentBlock = q.front();
	//						//std::cout << "current block is now " << currentBlock->getID() << std::endl;

	//					} else {
	//						currentBlock = s.top();
	//						s.pop();
	//					}
	//				}
	//			}
	//		}
	//		if (q.size() > 0) {

	//			q.pop();
	//		}
	//	}

	//	std::cout << currentBlock->getID() << std::endl;
	//	if (currentBlock->fallThroughFrom != nullptr && currentBlock->fallThroughFrom->visited == false) {
	//		if (currentBlock->fallThroughFrom->id != -1 && currentBlock->joinType != "while") {
	//			BasicBlock* myBB = currentBlock->fallThroughFrom;
	//			q.push(myBB);

	//		}
	//		if (currentBlock->fallThroughFrom->joinType != "while") {
	//			currentBlock->fallThroughFrom->visited = true;
	//		} else {
	//			currentBlock->fallThroughFrom->joinType = "";
	//			s.push(currentBlock->fallThroughFrom);
	//		}
	//	}
	//	if (currentBlock->branchFrom != nullptr && currentBlock->branchFrom->visited == false) {
	//		BasicBlock* myBB = currentBlock->branchFrom;
	//		//if (myBB->isWhile) {
	//		//	if (myBB->joinType == "while") {
	//		//		// not yet visited
	//		//		q.push(myBB);
	//		//	} else {
	//		//		// already visited
	//		//		if (myBB == s.top()) {
	//		//			//std::cout << myBB->getID() << s.top()->getID() << std::endl;
	//		//			s.pop();
	//		//			q.push(myBB);
	//		//		}
	//		//	}
	//		//} else {
	//		//	q.push(myBB);
	//		//}
	//		q.push(myBB);
	//		if (currentBlock->branchFrom->joinType != "while") {
	//			currentBlock->branchFrom->visited = true;

	//		} else {
	//			currentBlock->branchFrom->joinType = "";
	//			s.push(currentBlock->branchFrom);
	//		}
	//	}

	//	if (q.size() == 0 && s.size() > 0) {
	//		q.push(s.top());
	//		//std::cout << s.top()->getID() << " is the top of the stack" << std::endl;
	//		s.pop();
	//	}
	//}

	////if its a while join
	////	if it has been visited once
	////		if it is the top on the stack
	////			add to queue
	////  else
	////      add to queue
	////else
	////	add to queue
	//// 

	////BasicBlock* myBB = currentBlock->fallThroughFrom;
	////if (myBB->isWhile) {
	////	if (myBB->joinType == "while") {
	////		// not yet visited
	////		q.push(myBB);
	////	} else {
	////		// already visited
	////		if (myBB == s.top()) {
	////			q.push(myBB);
	////		}
	////	}
	////} else {
	////	q.push(myBB);
	////}


	////if (currentBlock->fallThroughFrom != nullptr) {
	////	traverseBasicBlocks(currentBlock->fallThroughFrom);
	////}
	////if (currentBlock->branchFrom != nullptr) {
	////	traverseBasicBlocks(currentBlock->branchFrom);
	////}

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
			// suss
			bb->unionLiveRanges(bb->fallThrough, "left");
			bb->unionLiveRanges(bb->branch, "right");
		}

	} else if (bb->conditionalBlockType == "ifThen-Then") {
		if (bb->branch == nullptr && bb->loop == nullptr) {
			// at the end of then in if-then struct
			bb->unionLiveRanges(bb->fallThrough, "left");
		} else {
			//std::cout << "None of the above cases" << std::endl;
			//std::cout << bb->bbRepr() << std::endl;
			bb->unionLiveRanges(bb->fallThrough, "");
			bb->unionLiveRanges(bb->branch, "right");
		}
	} else {
		if (bb->loop != nullptr) {
			// end of while body struct
			std::cout << "Found while body block" << std::endl;
			std::cout << bb->bbRepr(false) << std::endl;
			bb->unionLiveRanges(bb->loop, "right");
		} else {
			std::cout << "not while body block!!!" << std::endl;
			// need to handle cond with only then block here
			if (bb->splitType == "if") {
				if (bb->branch->joinType == "while") {
					bb->unionLiveRanges(bb->branch, "left");
				} else {
					bb->unionLiveRanges(bb->branch, "right"); // suss

				}
			} else {
				bb->unionLiveRanges(bb->branch, ""); // suss

			}
			//std::cout << "Issue is here" << std::endl;
			bb->unionLiveRanges(bb->fallThrough, "");
			bb->unionLiveRanges(bb->loop, "");

		}
	}


	//bb->unionLiveRanges(bb->fallThrough, "ft");
	//bb->unionLiveRanges(bb->branch, "br");
	//bb->unionLiveRanges(bb->loop, "loop");

	//bb->unionLiveRanges(bb->fallThrough, "ft");
	//bb->unionLiveRanges(bb->branch, "br");
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
		//if (node == nullptr) {
		//	// create a new igraph node
		//	node = new IGraphNode(kv.first, kv.first->id);
		//	iGraphNodes.push_back(node);
		//}
		std::vector<IGraphNode*> toErase;
		for (SSAValue* connectedToVal : kv.second) {
			IGraphNode* connectToNode = findInIGraphNodes(connectedToVal);
			//if (connectToNode == nullptr) {
			//	// need to create a new IGraphNode
			//	connectToNode = new IGraphNode(connectedToVal, connectedToVal->id);
			//	iGraphNodes.push_back(connectToNode);
			//}
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
	std::cout << "In coalesce phi" << std::endl;
	node->visited = true;
	SSAValue* val = node->initValue;
	SSAValue* phiOperand1 = val->operand1;
	SSAValue* phiOperand2 = val->operand2;


	if (phiOperand1 != nullptr && phiOperand1->op != CONST) {
		std::cout << "Op1 " << node->iGraphNodeRepr() << "\n" << phiOperand1->instCFGRepr() << std::endl;

		if (!checkInterferesWith(node, phiOperand1)) {
			IGraphNode* phiNode = nullptr;
			if (phiOperand1->op == PHI) {
				phiNode = findInIGraphNodes(phiOperand1);
				if (phiNode == nullptr) {
					std::cout << "Phi node is nullptr" << std::endl;
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
			//IGraphNode* phiOp1Node = findInIGraphNodes(phiOperand1);

			if (phiNode != nullptr) {
				for (IGraphNode* newInter : phiNode->connectedTo) {
					node->connectedTo.insert(newInter);
				}
				//iGraphNodes.erase(iGraphNodes.begin() + findIndexOfIGraphNode(phiOp1Node));
				//iGraphNodes.at(findIndexOfIGraphNode(phiOp1Node)) = nullptr;
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
		std::cout << "Op2 " << node->iGraphNodeRepr() << "\n" << phiOperand2->instCFGRepr() << std::endl;

		if (!checkInterferesWith(node, phiOperand2)) {
			std::cout << "no interference" << std::endl;
			IGraphNode* phiNode = nullptr;
			if (phiOperand1->op == PHI) {
				phiNode = findInIGraphNodes(phiOperand2);
				coalescePhi(phiNode, toDelete);
				if (phiNode == nullptr) {
					std::cout << "Phi node is nullptr" << std::endl;
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
			std::cout << "below conditional" << std::endl;
			node->values.insert(phiOperand2);
			//IGraphNode* phiOp2Node = findInIGraphNodes(phiOperand2);
			if (phiNode != nullptr) {
				std::cout << "node is nullptr" << std::endl;
				for (IGraphNode* newInter : phiNode->connectedTo) {
					node->connectedTo.insert(newInter);
				}
				//iGraphNodes.erase(iGraphNodes.begin() + findIndexOfIGraphNode(phiOp2Node));
				//iGraphNodes.at(findIndexOfIGraphNode(phiOp2Node)) = nullptr;
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
	//std::vector<IGraphNode*> nodes;
	//for (IGraphNode* node : iGraphNodes) {
	//	SSAValue* val = node->initValue;
	//	std::cout << val->instCFGRepr() << std::endl;
	//	if (val->op == PHI) {
	//		std::cout << "found phi!" << std::endl;

	//		SSAValue* phiOperand1 = val->operand1;
	//		SSAValue* phiOperand2 = val->operand2;
	//		if (phiOperand1->op != CONST) {
	//			std::cout << node->iGraphNodeRepr() << "\n" << phiOperand1->instCFGRepr() << std::endl;

	//			if (!checkInterferesWith(node, phiOperand1)) {
	//				node->values.insert(phiOperand1);
	//				IGraphNode* phiOp1Node = findInIGraphNodes(phiOperand1);
	//				for (IGraphNode* newInter : phiOp1Node->connectedTo) {
	//					node->connectedTo.insert(newInter);
	//				}
	//				//iGraphNodes.erase(iGraphNodes.begin() + findIndexOfIGraphNode(phiOp1Node));
	//				//iGraphNodes.at(findIndexOfIGraphNode(phiOp1Node)) = nullptr;
	//				nodes.push_back(phiOp1Node);
	//				addNewEdges(node, phiOp1Node);
	//				node->values.erase(val);
	//			} else {
	//				IGraphNode* phiOp1Node = findInIGraphNodes(phiOperand1);
	//				phiOp1Node->moveTarget = node;
	//			}
	//		}
	//		if (phiOperand2->op != CONST) {
	//			std::cout << node->iGraphNodeRepr() << "\n" << phiOperand2->instCFGRepr() << std::endl;

	//			if (!checkInterferesWith(node, phiOperand2)) {
	//				node->values.insert(phiOperand2);
	//				IGraphNode* phiOp2Node = findInIGraphNodes(phiOperand2);
	//				for (IGraphNode* newInter : phiOp2Node->connectedTo) {
	//					node->connectedTo.insert(newInter);
	//				}
	//				//iGraphNodes.erase(iGraphNodes.begin() + findIndexOfIGraphNode(phiOp2Node));
	//				//iGraphNodes.at(findIndexOfIGraphNode(phiOp2Node)) = nullptr;
	//				nodes.push_back(phiOp2Node);
	//				addNewEdges(node, phiOp2Node);
	//				node->values.erase(val);
	//			} else {
	//				IGraphNode* phiOp2Node = findInIGraphNodes(phiOperand2);
	//				phiOp2Node->moveTarget = node;
	//			}
	//		}
	//	}
	//}
	std::vector<IGraphNode*> toDelete;
	for (IGraphNode* node : iGraphNodes) {
		SSAValue* val = node->initValue;
		if (val->op == PHI && !node->visited) {
			coalescePhi(node, toDelete);
			std::cout << "finished coalesce phi" << std::endl;
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
	std::cout << "At end of pickNode" << std::endl;
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
	std::cout << "Pick colors" << std::endl;
}

void SSA::colorGraph() {
	std::cout << "Performing Graph Coloring" << std::endl;

	int numRegisters = 5;
	IGraphNode* node = pickNode(iGraphNodes);
	if (node == nullptr) {
		return;
	}
	int index = findIndexOfIGraphNode(node);
	std::cout << "index is " << index << std::endl;
	if (iGraphNodes.size() != 0 && index >= 0) {
		iGraphNodes.erase(iGraphNodes.begin() + index);
	}
	std::vector<IGraphNode*> addBackTo;
	//if (node != nullptr) {
	for (auto editNode : node->connectedTo) {
		if (editNode != nullptr) {
			addBackTo.push_back(editNode);
			editNode->connectedTo.erase(node);
		}
	}
	//}
	if (iGraphNodes.size() != 0) {

		colorGraph();
	} else {
		std::cout << "size is 0" << std::endl;
	}
	//if (node != nullptr) {
	iGraphNodes.push_back(node);

	//}
	for (auto editNode : addBackTo) {
		if (editNode != nullptr) {
			editNode->connectedTo.insert(node);
		}
	}
	//if (node != nullptr) {
	node->color = pickColor(node);

	//}
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
	// have to add move instructions in this function 
	// have to adjust basic block things
	int labelCount = 0;
	SSAValue* iter = instList;

	while (iter != nullptr) {
		if (iter->op == PHI) {
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
					SSAValue* currentTail = bbToAddMoveIn->tail;
					SSAValue* currentTailNext = currentTail->next;
					currentTail->next = moveInstr;
					moveInstr->prev = currentTail;
					moveInstr->next = currentTailNext;
					currentTailNext->prev = moveInstr;
					bbToAddMoveIn->tail = moveInstr;
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
			if (phiOperand2->regToMoveTo != 0) {
				BasicBlock* bbToAddMoveIn = phiOperand2->containingBB;
				SSAValue* currentTail = bbToAddMoveIn->tail;
				SSAValue* currentTailNext = currentTail->next;
				SSAValue* moveInstr = SSACreateMove(phiOperand2->regNum, phiOperand2->regToMoveTo);
				if (false) {
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
					//moveBB->head = moveInstr;
					//moveBB->tail = moveInstr;
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
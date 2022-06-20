#include "ssa.h"
#include "token.h"
#include <stdexcept>
#include <iostream>
#include <string>


// ssa value class

std::string SSAValue::getTextForEnum(int enumVal) {
	return opcodeEnumStrings[enumVal];
}

std::string SSAValue::formatOperand(SSAValue* operand) {
	return "(" + std::to_string(operand->id) + ")";
}
void SSAValue::instRepr() {

	if (op == CONST) {
		std::cout << id << ": CONST #" << constValue << std::endl;
	} else if (op == BRA || op == write) {
		std::cout << id << ": " << getTextForEnum(op) << formatOperand(operand1) << std::endl;
	} else if (op == NOP || op == read) {
		std::cout << id << ": " << getTextForEnum(op) << std::endl;
	} else {
		std::cout << id << ": " << getTextForEnum(op) << formatOperand(operand1) << formatOperand(operand2) << std::endl;
	}
}

std::string SSAValue::getType() {
	std::string type;
	if (isVar) {
		type = "VAR";
	} else {
		type = "CONST";
	}
	return type;
}

std::string SSAValue::getNameType() {
	std::string info = "; " + getType() + " " + name;
	if (operand1 != nullptr) {
		info += ", lArg: " + operand1->getType() + " " + operand1->name;
	}
	if (operand2 != nullptr) {
		info += ", rArg: " + operand2->getType() + " " + operand2->name;
	}
	return info;
}

void SSAValue::instReprWNames() {

	if (op == CONST) {
		std::cout << id << ": CONST #" << constValue << getNameType() << std::endl;
	} else if (op == BRA || op == write) {
		std::cout << id << ": " << getTextForEnum(op) << formatOperand(operand1) << getNameType() << std::endl;
	} else if (op == NOP || op == read) {
		std::cout << id << ": " << getTextForEnum(op) << getNameType() << std::endl;
	} else {
		std::cout << id << ": " << getTextForEnum(op) << formatOperand(operand1) << formatOperand(operand2) << getNameType() << std::endl;
	}
}

std::string SSAValue::instCFGRepr() {
	std::string output = "";
	if (op == CONST) {
		output = std::to_string(id) + ": CONST #" + std::to_string(constValue);
	} else if (op == BRA || op == write) {
		output = std::to_string(id) + ": " + getTextForEnum(op) + formatOperand(operand1);
	} else if (op == NOP || op == read) {
		output = std::to_string(id) + ": " + getTextForEnum(op);
	} else {
		output = std::to_string(id) + ": " + getTextForEnum(op) + formatOperand(operand1) + formatOperand(operand2);
		
	}
	return output;
}

void SSAValue::setNameType(std::string ssaName, bool ssaType) {
	name = ssaName;
	isVar = ssaType;
}

// ssa class

int SSA::maxID = 0;
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
	if (operation >= ADDOP && operation <= DIVOP) {
		// these ops can be involved in CSE
		// search in symbol table for dominating inst with same opcode
		// then search up the linked list looking for an inst that has same params
		prevDomWithOpcode = findPrevDomWithOpcode(operation);

		SSAValue* iter = prevDomWithOpcode;
		while (iter != nullptr) {
			//std::cout << "SSACreate prevDomWithOpcode" << iter->instCFGRepr() << std::endl;
			if (iter->operand1 == x && iter->operand2 == y) {

				// found common subexpression
				return iter;
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
	return result;

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

void SSA::SSACreate(opcode operation, SSAValue* y) {
	SSAValue* result = new SSAValue();
	result->id = maxID++;
	result->op = operation;
	result->operand1 = y;
	addInOrder(result);
	addSSAValue(result);
}

SSAValue* SSA::SSACreateConst(int constVal) {

	SSAValue* val = findConst(constVal);
	if (val == nullptr) {
		SSAValue* ssaConst = new SSAValue();
		ssaConst->op = CONST;
		ssaConst->id = maxID++;
		ssaConst->constValue = constVal;
		addInOrder(ssaConst);
		addConst(constVal, ssaConst);
		addSSAValue(ssaConst);
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

int SSA::getTailID() {
	return instTail->id;
}

SSAValue* SSA::getTail() {
	return instTail;
}

void SSA::setInstTail(SSAValue* newTail) {
	instTail = newTail;
}

SSA::SSA() {
	instListLength = 0;
	instTail = new SSAValue();
	scopeDepth = 0;
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
	symTable.back().insert_or_assign(name, val);
	symTableCopy.back().insert_or_assign(name, val);


}

SSAValue* SSA::findSymbol(std::string name) {
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

}

// constTable functions

void SSA::addConst(int constVal, SSAValue* constSSAVal) {
	constTable.insert({ constVal, constSSAVal });

}

SSAValue* SSA::findConst(int constVal) {
	try {
		SSAValue* val = constTable.at(constVal);
		return val;
	} catch (std::out_of_range& oor) {
		return nullptr;
	}

}

// ssa debugging functions
std::string SSA::outputSSA() {
	std::string output = "";
	SSAValue* current = instList;
	while (current != nullptr) {
		output += current->instCFGRepr() + "\n";
		current = current->next;
	}
	return output;
}


void SSA::printSSA() {
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

std::string SSA::genBBStart(int bbID) {
	std::string bbIDStr = std::to_string(bbID);
	std::string currentBB = "bb" + bbIDStr + "[shape=record, label=\" < b > BB" + bbIDStr + " | {";
	return currentBB;
}

std::tuple<std::vector<BasicBlock*>, std::vector<BBEdge*>> SSA::genBasicBlocks() {
	/*
		if a branch instruction is found
			close current bb (include branch inst in current bb)
			create new bb
			store branch-to id for new block
		else
		    if id was previously a branch-to id
			    close current bb
				create new bb
			else
			    just add inst to bb
	*/

	SSAValue* current = instList; 
	SSAValue* prevCurrent = current;
	std::unordered_map<int, BasicBlock*> branchToBlocks; // inst ids for when to create new bb
	std::vector<BasicBlock*> basicBlocks;
	std::vector<BBEdge*> bbEdges;

	BasicBlock* currentBB = nullptr;
	int bbID = 1;


	while (current != nullptr) {
		if (currentBB == nullptr) {
			currentBB = new BasicBlock(true);
			currentBB->setBBID(1);
			currentBB->setHead(current);
			currentBB->setTail(current);
		}
		if (current->op <= 17 && current->op >= 11) {
			// any branch inst
			// close current block, create new block, and store branch-to id for new block later
			currentBB->setTail(current);
			bbID += 1;
			basicBlocks.push_back(currentBB);





			currentBB = new BasicBlock(true);
			currentBB->setHead(current->next);
			currentBB->setTail(current);
			currentBB->setBBID(bbID);

			BBEdge* newEdge = new BBEdge(basicBlocks.back(), currentBB, "ft");
			bbEdges.push_back(newEdge);

			



			// at this point, currentBB is FT block (edge to it from split block must be ft edge)

			int newBlockID;
			if (current->op == BRA) {
				// bra inst
				newBlockID = current->operand1->id;

			} else {
				// all other branch insts (12 <= op <= 17)
				newBlockID = current->operand2->id;

			}
			// before creating branch block, first check that the inst does not exist above
			bool found = false;
			BasicBlock* jumpToBlock = nullptr;
			for (BasicBlock* bb : basicBlocks) {
				SSAValue* bbHead = bb->getHead();
				SSAValue* bbTail = bb->getTail();
				while (bbHead != bbTail->next) {
					if (bbHead->id == newBlockID) {
						found = true;
						jumpToBlock = bb;
						break;
					}

					bbHead = bbHead->next;
				}
			}
			std::cout << "FOUND is " << found << std::endl;
			if (!found) {
				BasicBlock* branchBlock = new BasicBlock(false);
				branchToBlocks.insert({ newBlockID, branchBlock });
				BBEdge* newEdge = new BBEdge(basicBlocks.back(), branchBlock, "br");
				bbEdges.push_back(newEdge);
			} else {
				BBEdge* newEdge = new BBEdge(basicBlocks.back(), jumpToBlock, "br");
				bbEdges.push_back(newEdge);

			}

		} else {
			bool mustCreateNewBlock = false;
			// check if current inst id needs to be start of new bb
			BasicBlock* branchToBlock = nullptr;
			for (auto kv : branchToBlocks) {
				if (current->id == kv.first) {
					branchToBlock = kv.second;
					break;
				}
			}
			if (branchToBlock != nullptr) {
				basicBlocks.push_back(currentBB);
				bbID += 1;
				currentBB = branchToBlock;
				currentBB->setBBID(bbID);
				currentBB->setHead(current);
				currentBB->setTail(current);
				// at this point, new block is a branch-to block, must have edge type of branch
			} else {
				// just add instruction to current block
				if (currentBB->getHead() == nullptr) {
					currentBB->setHead(current);
				}
				currentBB->setTail(current);
			}
		}
		prevCurrent = current;
		current = current->next;
	}
	currentBB->setTail(prevCurrent); // questionable
	basicBlocks.push_back(currentBB);

	return std::make_tuple(basicBlocks, bbEdges);
	//std::tuple<std::vector<BasicBlock*>, std::vector<BBEdge*>> info;
	//info->basicBlocks = basicBlocks;
	//info->bbEdges = bbEdges;
	//return info;
}

void SSA::gen() {
	std::tuple<std::vector<BasicBlock*>, std::vector<BBEdge*>> info = genBasicBlocks();
	std::vector<BasicBlock*> basicBlocks = std::get<0>(info);
	std::vector<BBEdge*> bbEdges = std::get<1>(info);
	std::cout << "digraph prog {" << std::endl;

	for (BasicBlock* bb: basicBlocks) {
		std::cout << bb->bbRepr() << std::endl;
	}

	for (BBEdge* edge : bbEdges) {
		std::cout << edge->bbEdgeRepr() << std::endl;
	}

	std::cout << "}" << std::endl;
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

BasicBlock::BasicBlock(bool ft) {
	head = nullptr;
	tail = nullptr;
	id = 0;
	incomingEdgeIsFT = ft;
}

SSAValue* BasicBlock::getHead() {
	return head;
}

void BasicBlock::setHead(SSAValue* bbHead) {
	head = bbHead;
}

SSAValue* BasicBlock::getTail() {
	return tail;
}

void BasicBlock::setTail(SSAValue* bbTail) {
	tail = bbTail;
}

int BasicBlock::getID() {
	return id;
}

void BasicBlock::setBBID(int bbID) {
	id = bbID;
}

std::string BasicBlock::bbRepr() {
	SSAValue* current = head;
	SSAValue* stopAt = tail->next;
	std::string bbIDStr = std::to_string(id);
	std::string bbString = "bb" + bbIDStr + "[shape=record, label=\" < b > BB" + bbIDStr + " | {";
	while (current != stopAt) {
		bbString = bbString + current->instCFGRepr() + "|";
		current = current->next;
	}
	bbString.pop_back();
	bbString = bbString + "}\"];";
	return bbString;
}


// BBEdge
BBEdge::BBEdge(BasicBlock* fromBlock, BasicBlock* toBlock, std::string edgeType) {
	from = fromBlock;
	to = toBlock;
	type = edgeType;
}

BasicBlock* BBEdge::getFrom() {
	return from;
}
BasicBlock* BBEdge::getTo() {
	return to;
}
std::string BBEdge::getType() {
	return type;
}

std::string BBEdge::bbEdgeRepr() {
	int fromID = from->getID();
	int toID = to->getID();

	std::string fromIDStr = std::to_string(fromID);
	std::string toIDStr = std::to_string(toID);


	std::string out = "bb" + fromIDStr + ":s -> bb" + toIDStr + ":n [label=\"" + type + "\"];";
	return out;
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

	constTable = std::unordered_map<int, SSAValue*>();
}
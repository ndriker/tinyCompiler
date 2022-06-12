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
	SSAValue* result = new SSAValue();
	result->id = maxID++;
	result->op = operation;
	result->operand1 = x;
	result->operand2 = y;
	addSSAValue(result);
	return result;

}
SSAValue* SSA::SSACreateWhilePhi(SSAValue* x, SSAValue* y) {
	SSAValue* result = new SSAValue();
	result->id = maxID++;
	result->op = PHI;
	result->operand1 = x;
	result->operand2 = y;
	return result;
}

void SSA::SSACreate(opcode operation, SSAValue* y) {
	SSAValue* result = new SSAValue();
	result->id = maxID++;
	result->op = operation;
	result->operand1 = y;
	addSSAValue(result);
}

SSAValue* SSA::SSACreateConst(int constVal) {

	SSAValue* val = findConst(constVal);
	if (val == nullptr) {
		SSAValue* ssaConst = new SSAValue();
		ssaConst->op = CONST;
		ssaConst->id = maxID++;
		ssaConst->constValue = constVal;
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

	scopeDepth = scopeDepth + 1;
}

std::unordered_map<std::string, SSAValue*> SSA::exitScope() {
	std::unordered_map<std::string, SSAValue*> lastScope = symTable.back();
	symTable.pop_back();
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



void SSA::printSSA() {
	SSAValue* current = instList;
	while (current != nullptr) {
		current->instRepr();
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


void SSA::generateDotLang() {
	
	std::unordered_map<int, BBEdge*> branchEdgeMap;
	std::unordered_map<int, BBEdge*> fallThroughEdgeMap;

	
	SSAValue* current = instList;
	SSAValue* prevCurrent = nullptr;
	std::vector<int> newBlockIDs;
	std::vector<std::string> bbStrings;
	std::string currentBB = "";
	int bbID = 1;

	bool isBlockNew = false;

	while (current != nullptr) {
		if (currentBB == "") {
			currentBB = "bb1 [shape=record, label=\" < b > BB1 | {";
		}



		if (current->op <= 17 && current->op >= 11) {
			// branch inst
			// close current block, create new block, and store branch-to id for new block later
			BBEdge* newBranchEdge = new BBEdge(bbID);
			BBEdge* newFTEdge = new BBEdge(bbID);
			currentBB = currentBB + current->instCFGRepr() + "}\"];";

			fallThroughEdgeMap.insert({ current->id + 1, newFTEdge });
			current->bbID = bbID;

			bbStrings.push_back(currentBB);
			bbID += 1;
			std::string bbIDStr = std::to_string(bbID);
			currentBB = "bb" + bbIDStr + "[shape=record, label=\" < b > BB" + bbIDStr + " | {";
			isBlockNew = true;
			int newBlockID;
			if (current->op == 11) {
				// bra inst
				newBlockID = current->operand1->id;

			} else {
				// all other branch insts (12 <= op <= 17)
				newBlockID = current->operand2->id;
			}
			
			branchEdgeMap.insert({ newBlockID, newBranchEdge });
			newBlockIDs.push_back(newBlockID);

		} else {
			bool mustCreateNewBlock = false;
			for (int id : newBlockIDs) {
				if (current->id == id) {
					mustCreateNewBlock = true;
					break;
				}
			}
			if (mustCreateNewBlock) {
				if (!isBlockNew) {
					currentBB.pop_back();
					currentBB = currentBB + "}\"];";
					bbStrings.push_back(currentBB);
					bbID += 1;
					std::string bbIDStr = std::to_string(bbID);
					currentBB = "bb" + bbIDStr + "[shape=record, label=\" < b > BB" + bbIDStr + " | {";

				}
				currentBB = currentBB + current->instCFGRepr() + "|";
				current->bbID = bbID;
				isBlockNew = false;
			} else {
				// just add instruction to current block
				currentBB = currentBB + current->instCFGRepr() + "|";
				current->bbID = bbID;

				isBlockNew = false;
			}

		}
		prevCurrent = current;
		current = current->next;

	}
	currentBB.pop_back();
	currentBB = currentBB + "}\"];";
	bbStrings.push_back(currentBB);

	current = instList;
	while (current != nullptr) {

		try {
			BBEdge* bEdge = branchEdgeMap.at(current->id);
			bEdge->setToBlockID(current->bbID);
		} catch (std::out_of_range& oor) {
			// intentionally left blank
		}

		try {
			BBEdge* fEdge = fallThroughEdgeMap.at(current->id);
			fEdge->setToBlockID(current->bbID);

		} catch (std::out_of_range& oor) {
			 //intentionally left blank
		}
		current = current->next;
	}



	std::cout << "digraph G {" << std::endl;
	for (std::string s : bbStrings) {
		std::cout << s << std::endl;
	}



	std::unordered_map<int, int> visited;
	for (auto kv : branchEdgeMap) {
		BBEdge* edge = kv.second;
		std::cout << "bb" << edge->getFrom() << ":s -> bb" << edge->getTo() << ":n;" << std::endl;
		visited.insert({ edge->getFrom(), edge->getTo() });
	}
	for (auto kv : fallThroughEdgeMap) {
		BBEdge* edge = kv.second;
		try {
			int to = visited.at(edge->getFrom());
			if (to != edge->getTo()) {
				std::cout << "bb" << edge->getFrom() << ":s -> bb" << edge->getTo() << ":n;" << std::endl;

			}
		} catch (std::out_of_range& oor) {
			std::cout << "bb" << edge->getFrom() << ":s -> bb" << edge->getTo() << ":n;" << std::endl;

		}
	}


	std::cout << "}" << std::endl;
	for (auto kv : branchEdgeMap) {
		BBEdge* edge = kv.second;
		std::cout << kv.first << " " << edge->getFrom() << " " << edge->getTo() << std::endl;
	}
}


// BasicBlock
int BasicBlock::maxID = 0;


void BasicBlock::setHead(SSAValue* head) {
	headInst = head;
}

void BasicBlock::setTail(SSAValue* tail) {
	tailInst = tail;
}

void BasicBlock::setBlockLabel(std::string label) {
	blockLabel = label;
}

// BBEdge
BBEdge::BBEdge(int from) {
	fromBlockID = from;
	toBlockID = -1;
	edgeComplete = false;

}

void BBEdge::setToBlockID(int to) {
	toBlockID = to;
	edgeComplete = true;
}

int BBEdge::getFrom() {
	return fromBlockID;
}

int BBEdge::getTo() {
	return toBlockID;
}
#include "parser.h"
#include <iostream>
#include <vector>
#include "token.h"
#include <stdexcept>


/*
	NOTES:
		Only call next() when token needs to be consumed
		For handling varRef inside of factor, as an example,
			first you need to check the token type. If it is
			an ident, then you must call varRef, and only consume
			the token inside of varRef

		Calling next() before being inside of the varRef function
			will 1. lead to the wrong tree, and 2. lead to incorrect
			production
*/

void Parser::setTokens(std::vector<Token> inTokens) {
	tokens = inTokens;
	currPos = 0;

}

void Parser::setDebug(bool debugMode) {
	debug = debugMode;
}


void Parser::next() {
	if (currPos < tokens.size() - 1) {
		currPos += 1;
		sym = tokens[currPos].getType();
	}
}

std::string Parser::getCurrentValue() {
	return tokens[currPos].getValue();
}



void Parser::error(std::string errorMessage) {
	std::cout << "Parser Error: " << errorMessage << std::endl;
}

std::string Parser::createIndent() {
	return std::string(3 * currentPrintIndent, ' ');
}

void Parser::incPrintInd() {
	currentPrintIndent += 1;
}

void Parser::decPrintInd() {
	currentPrintIndent -= 1;
}

void Parser::printItem(std::string item, std::string value) {
	if (debug) {
		incPrintInd();
		std::cout << createIndent() << item << " " << value << std::endl;
		decPrintInd();
	}
}

void Parser::startPrintBlock(std::string blockName) {
	if (debug) {
		incPrintInd();
		std::cout << createIndent() << blockName << std::endl;
	}
}

SSAValue* Parser::varRef() {
	startPrintBlock("Var Ref");

	std::string ident = getCurrentValue();
	printItem("Identifier", ident);
	SSAValue* identInst = ssa->findSymbol(ident);
	//identInst->setNameType(ident, true);

	next(); // consume ident
	decPrintInd();
	return identInst;
}

std::tuple<SSAValue*, std::string> Parser::factor() {
	startPrintBlock("Factor");
	std::tuple<SSAValue*, std::string> leftTuple;
	SSAValue* left;
	std::string name = "";
	if (sym == IDENT) {
		name = getCurrentValue();
		left = varRef();
		std::cout << "Name from factor " << name << std::endl;
	}
	else if (sym == NUMBER) {
		std::string numStr = getCurrentValue();
		printItem("Number", numStr);
		int num = stoi(numStr);
		left = ssa->SSACreateConst(num);
		//left->setNameType("#" + numStr, false);
		next();
	}
	else if (sym == L_PAREN) {
		printItem("Left Paren");
		next();
		leftTuple = expression();
		left = std::get<0>(leftTuple);
		name = std::get<1>(leftTuple);

		if (sym == R_PAREN) {
			printItem("Right Paren");
			next();
		}
		else {
			error("Mismatched Parentheses");
		}
	}
	else if (sym == CALL) {
		left = funcCall();
		// TODO
		//left = new SSAValue();
	}
	else {
		error("No Production Exists for your input");
		// TODO
		left = new SSAValue();
	}
	
	decPrintInd();
	//return left;
	return std::make_tuple(left, name);
}

std::tuple<SSAValue*, std::string> Parser::term() {
	startPrintBlock("Term");
	bool wasMul;


	std::tuple<SSAValue*, std::string> leftTuple;
	std::tuple<SSAValue*, std::string> rightTuple;



	leftTuple = factor();
	SSAValue* left = std::get<0>(leftTuple);
	std::string leftName = std::get<1>(leftTuple);

	while ((sym == MUL) || (sym == DIV)) {
		wasMul = (sym == MUL) ? true : false;
		printItem(getTextForEnum(sym));
		next();
		rightTuple = factor();
		SSAValue* right = std::get<0>(rightTuple);
		std::string rightName = std::get<1>(rightTuple);
		if (wasMul) {
			left = ssa->SSACreate(MULOP, left, right);

		} else {
			// sym == div
			left = ssa->SSACreate(DIVOP, left, right);
		}
		left->setOpNames(leftName, rightName);
		//if (loopCount < 1) {
		//	left->setOpNames(leftName, rightName);
		//} else {
		//	left->setOpNames("", rightName);
		//}

		leftName = "_";

	}

	decPrintInd();
	//return left;
	return std::make_tuple(left, leftName);
}

std::tuple<SSAValue*, std::string> Parser::expression() {
	startPrintBlock("Expression");
	bool wasAdd;
	std::tuple<SSAValue*, std::string> leftTuple;
	std::tuple<SSAValue*, std::string> rightTuple;

	leftTuple = term();
	SSAValue* left = std::get<0>(leftTuple);
	std::string leftName = std::get<1>(leftTuple);
	while ((sym == ADD) || (sym == SUB)) {
		wasAdd = (sym == ADD) ? true : false;
		printItem(getTextForEnum(sym));
		next();

		rightTuple = term();
		SSAValue* right = std::get<0>(rightTuple);
		std::string rightName = std::get<1>(rightTuple);
		if (wasAdd) {
			left = ssa->SSACreate(ADDOP, left, right);
		} else {
			// sym == sub
			left = ssa->SSACreate(SUBOP, left, right);
		}

		left->setOpNames(leftName, rightName);
		//if (loopCount < 1) {
		//	left->setOpNames(leftName, rightName);
		//} else {
		//	left->setOpNames("", rightName);
		//}
		leftName = "_";
	}
	decPrintInd();
	return std::make_tuple(left, leftName);
}

SSAValue* Parser::relation() {
	startPrintBlock("Relation");
	std::tuple<SSAValue*, std::string> leftTuple;
	std::tuple<SSAValue*, std::string> rightTuple;
	SSAValue* left;
	SSAValue* right = nullptr;
	std::string leftName, rightName;
	tokenType symCopy;
	leftTuple = expression();
	left = std::get<0>(leftTuple);
	leftName = std::get<1>(leftTuple);
	if ((sym >= 18) || (sym <= 23)) {
		symCopy = sym;
		printItem(getTextForEnum(sym));

		// take sym to know which relational op it is
		next();
		rightTuple = expression();
		right = std::get<0>(rightTuple);
		rightName = std::get<1>(rightTuple);
	}
	SSAValue* cmp = ssa->SSACreate(CMP, left, right);
	cmp->setOpNames(leftName, rightName);

	elseHead = ssa->SSACreateNop();
	SSAValue* rel = ssa->SSACreate(ssa->convertBr(symCopy), cmp, elseHead);
	decPrintInd();
	return rel;
}

void Parser::assignment() {
	startPrintBlock("Assignment");
	std::string identName;
	std::tuple<SSAValue*, std::string> leftTuple;
	SSAValue* left;
	if (sym == LET) {
		printItem(getTextForEnum(sym));
		next(); // consume LET
		if (sym == IDENT) {
			identName = getCurrentValue();
			printItem(getTextForEnum(sym), identName);

			next(); // consume IDENT

			if (sym == ASSIGN) {
				printItem(getTextForEnum(sym));

				next(); // consume ASSIGN
				leftTuple = expression();
				left = std::get<0>(leftTuple);
				ssa->addSymbol(identName, left);
				//if (left->op != CONST) {
				//	left->setNameType(identName, true);
				//}

			} else {
				error("Arrow Error");
			}

		} else {
			error("Assignment must have identifier");
		}
	} else {
		error("Assignment must start with 'let'");

	}
	decPrintInd();

}

SSAValue* Parser::funcCall() {
	startPrintBlock("Function Call");
	bool isVoid = false;
	std::vector<std::string> formalParams;
	std::vector<SSAValue*> argVals;
	if (sym == CALL) {
		printItem(getTextForEnum(sym));
		next();
		if (sym == IDENT) {
			printItem(getTextForEnum(sym), getCurrentValue());
			std::string ident = getCurrentValue();
			funcDescriptor funcToCall = funcDescriptors.at(ident);
			isVoid = funcToCall.isVoid;
			formalParams = funcToCall.formalParams;
			next();
			

			if (sym == L_PAREN) {
				// do the with paren version
				printItem(getTextForEnum(sym));
				next(); // consume left paren
				if (sym == R_PAREN) {
					// case of no arguments

					printItem(getTextForEnum(sym));
					next(); // consume right paren

					//if (ident == "InputNum") {
					//	return ssa->SSACreate(read, nullptr, nullptr);
					//}
				} else {
					std::tuple<SSAValue*, std::string> resultTuple = expression();
					SSAValue* result = std::get<0>(resultTuple);
					argVals.push_back(result);

					while (sym == COMMA) {
						printItem(getTextForEnum(sym));
						next(); // consume comma
						std::tuple<SSAValue*, std::string> resultTuple = expression();
						result = std::get<0>(resultTuple);
						argVals.push_back(result);

					}
					if (sym == R_PAREN) {
						printItem(getTextForEnum(sym));
						next();
						if (ident == "OutputNum") {
							ssa->SSACreate(write, result);
							return nullptr;
						}
					} else {
						error("Mismatched Parentheses");
					}
				}
			}
			if (isVoid) {
				if (ident == "OutputNewLine") {
					ssa->SSACreate(writeNL, nullptr, nullptr);
				} else if (ident == "OutputNum") {
					ssa->SSACreate(write, argVals.at(0));
				} else {
					ssa->SSACreateCall(ident, argVals, formalParams);
				}
				return nullptr;
			} else {
				if (ident == "InputNum") {
					return ssa->SSACreate(read, nullptr, nullptr);
				} else {
					return ssa->SSACreateCall(ident, argVals, formalParams);
				}
			}

		} else {
			error("Call must have identifier");
		}
	} else {
		error("Call must have 'call'");
	}
	decPrintInd();
}


void Parser::ifStatement() {
	startPrintBlock("If Statement");

	SSAValue* savedElseHead = elseHead;
	SSAValue* savedJoinBlockHead = joinBlockHead;
	std::unordered_map<std::string, SSAValue*> savedPhiMap = phiMap;
	BasicBlock* savedJoinBlock = joinBlock;
	joinBlock = ssa->createBlock();
	ssa->setJoinType(joinBlock, "if");

	//ssa->connectFT(joinBlock, savedJoinBlock);

	if (sym == IF) {
		printItem(getTextForEnum(sym));
		next();
		relation();
		BasicBlock* splitBlock = ssa->getContext();
		// remember split block
		// create fall through block
		// connect split to fall through block
		// context switch to fall through block
		BasicBlock* thenBlock;
		if (sym == THEN) {
			printItem(getTextForEnum(sym));
			next();

			phiMap = std::unordered_map<std::string, SSAValue*>();
			thenBlock = ssa->createContext();
			thenBlock->dom = splitBlock;
			statSequence();
			ssa->connectFT(splitBlock, thenBlock);
			std::unordered_map<std::string, SSAValue*> thenPhiMap = phiMap;


			if (sym == ELSE) {
				// remember the then block
				// create branch block
				// connect split to branch block
				// context switch to branch block
				joinBlockHead = ssa->SSACreateNop();
				ssa->SSACreate(BRA, joinBlockHead);

				BasicBlock* elseBlock = ssa->createContext();
				elseBlock->dom = splitBlock;
				ssa->updateNop(elseHead);
				printItem(getTextForEnum(sym));
				next();

				phiMap = std::unordered_map<std::string, SSAValue*>();
				
				statSequence();

				ssa->connectBR(splitBlock, elseBlock);
				
				std::unordered_map<std::string, SSAValue*> elsePhiMap = phiMap;
				
				ssa->initBlock(joinBlock);
				ssa->setContext(joinBlock);
				ssa->updateNop(joinBlockHead);
				joinBlock->dom = splitBlock;


				//joinBlock = ssa->createContext();
				ssa->connectBR(thenBlock, joinBlock);
				ssa->connectFT(elseBlock, joinBlock);

				for (auto kv : thenPhiMap) {
					try {
						SSAValue* val = elsePhiMap.at(kv.first);
						ssa->SSACreate(PHI, kv.second, val);
						elsePhiMap.erase(kv.first);
						ssa->addSymbol(kv.first, ssa->getTail());
					}
					catch (std::out_of_range& oor) {
						SSAValue* prevOccur = ssa->findSymbol(kv.first);
						ssa->SSACreate(PHI, kv.second, prevOccur);
						ssa->addSymbol(kv.first, ssa->getTail());

					}
				}
				for (auto kv : elsePhiMap) {
					SSAValue* prevOccur = ssa->findSymbol(kv.first);
					ssa->SSACreate(PHI, prevOccur, kv.second);
					ssa->addSymbol(kv.first, ssa->getTail());

				}
			} else {
				// if there is no else, elseHead becomes 
				//		the head of the join block, no branch needed
				//		just fall through
				ssa->initBlock(joinBlock);
				ssa->setContext(joinBlock);
				joinBlock->dom = splitBlock;
				
				
				//joinBlock = ssa->createContext();
				ssa->updateNop(elseHead);
				ssa->connectFT(thenBlock, joinBlock);
				ssa->connectBR(splitBlock, ssa->getContext());
				for (auto kv : thenPhiMap) {
					SSAValue* prevOccur = ssa->findSymbol(kv.first);
					ssa->SSACreate(PHI, kv.second, prevOccur);
					ssa->addSymbol(kv.first, ssa->getTail());

				}
			}
			if (sym == FI) {
				printItem(getTextForEnum(sym));
				next();
			} else {
				error("No 'fi' found for conditional");
			}
		} else {
			error("Conditional statement must have 'then'");
		}
	} else {	
		error("If statement must have 'if'");
	}
	//std::cout << "TAIL IS " << joinBlock->tail->instCFGRepr() << std::endl;
	//if (joinBlock->tail->op == BRA) {

	//	ssa->connectBR(joinBlock, savedJoinBlock);
	//} else {
	//	ssa->connectFT(joinBlock, savedJoinBlock);
	//}
	if (savedJoinBlock != nullptr && savedJoinBlock->joinType != "while") {
		ssa->connectFT(joinBlock, savedJoinBlock);
	}

	elseHead = savedElseHead;
	joinBlockHead = savedJoinBlockHead;
	joinBlock = savedJoinBlock;
	phiMap = savedPhiMap;
	decPrintInd();
}

void Parser::whileStatement() {
	startPrintBlock("While Statement");

	SSAValue* savedElseHead = elseHead;
	SSAValue* savedJoinBlockHead = joinBlockHead;
	std::unordered_map<std::string, SSAValue*> savedPhiMap = phiMap;
	BasicBlock* savedJoinBlock = joinBlock;
	bool savedInWhile = ssa->getInWhile();
	//joinBlock = ssa->createBlock();
	//ssa->initBlock(joinBlock);
	//ssa->setContext(joinBlock);
	ssa->setInWhile(true);
	BasicBlock* followBlock = nullptr;
	joinBlock = ssa->getContext();
	ssa->setJoinType(joinBlock, "while");
	if (sym == WHILE) {
		printItem(getTextForEnum(sym));
		next();
		SSAValue* branchInst = relation();
		BasicBlock* whileBodyBlock = ssa->createContext();
		whileBodyBlock->dom = joinBlock;
		joinBlockHead = ssa->SSACreateNop();

		if (sym == DO) {
			printItem(getTextForEnum(sym));
			next();
			phiMap = std::unordered_map<std::string, SSAValue*>();
			SSAValue* whileBodyHead = ssa->SSACreateNop();
			ssa->updateNop(whileBodyHead);
			statSequence();
			ssa->SSACreate(BRA, joinBlockHead);
			SSAValue* whileBodyTail = ssa->getTail();

			// our design choice for variables is that they are always global (i.e. can't create a variable local to one specific scope)
			// as a result, any assignment in a basic block generates a phi instruction, even if the variable was not previously defined 
			//     in outer scopes

			// at this point, we are in the scope immediately outside of while body
			// for every assignment inside of while body
			//     create a phi instruction in the join block with assignment from while body on left, and most recent outer assignment on right
			//	
			std::unordered_map<std::string, SSAValue*> whilePhiMap = phiMap; // scope symbol table of while body
			ssa->connectFT(joinBlock, whileBodyBlock);
			ssa->connectBR(ssa->getContext(), joinBlock);
			ssa->setContext(joinBlock, true);
			ssa->updateNop(joinBlockHead);
			std::cout << joinBlockHead->instCFGRepr() << std::endl;

			SSAValue* joinBlockTail = joinBlockHead;
			                   // argName         // prevId       // changeToID
			std::unordered_map<std::string, std::tuple<SSAValue*, SSAValue*>> idChanges;
			//std::unordered_map<SSAValue*, SSAValue*> idChanges;
			for (auto kv : whilePhiMap) {
				SSAValue* prevOccur = ssa->findSymbol(kv.first); // outer scope value
				SSAValue* phiInst = ssa->SSACreate(PHI, prevOccur, kv.second); 
				std::tuple<SSAValue*, SSAValue*> replaceInfo = std::make_tuple(prevOccur, phiInst);
				idChanges.insert_or_assign(kv.first, replaceInfo); 
				joinBlockTail = phiInst;
				ssa->addSymbol(kv.first, ssa->getTail());
			}

			SSAValue* aboveCmpInst = branchInst->prev->prev;


			SSAValue* cmpInst = branchInst->prev;
			SSAValue* cmpLeftOp = cmpInst->operand1;
			SSAValue* cmpRightOp = cmpInst->operand2;
			//std::cout << "cmp left op ";
			//cmpLeftOp->instRepr();
			try {
				std::string cmpLeftName = cmpInst->op1Name;
				std::tuple<SSAValue*, SSAValue*> replaceInfo = idChanges.at(cmpLeftName);
				if (cmpLeftOp == std::get<0>(replaceInfo)) {
					cmpInst->operand1 = std::get<1>(replaceInfo);
				}
			} catch (std::out_of_range& oor) {
				// intentionally left blank
			}

			try {
				std::string cmpRightName = cmpInst->op2Name;
				std::tuple<SSAValue*, SSAValue*> replaceInfo = idChanges.at(cmpRightName);
				if (cmpRightOp == std::get<0>(replaceInfo)) {
					cmpInst->operand2 = std::get<1>(replaceInfo);
				}
			}
			catch (std::out_of_range& oor) {
				// intentionally left blank
			}


			//try {
			//	cmpInst->operand1 = idChanges.at(cmpInst->operand1);
			//} catch (std::out_of_range& oor) {
			//	std::cout << "Most likely infinite loop, check that you are modifying the loop control variable." << std::endl;
			//	throw std::logic_error("Most likely infinite loop, check that you are modifying the loop control variable.");
			//}


			aboveCmpInst->next = joinBlockHead;
			joinBlockHead->prev = aboveCmpInst;



			joinBlockTail->next = cmpInst;
			cmpInst->prev = joinBlockTail;

			branchInst->next = whileBodyHead;
			whileBodyHead->prev = branchInst;

			ssa->setInstTail(whileBodyTail);

			followBlock = ssa->createContext(); // also does setContext
			ssa->connectBR(joinBlock, followBlock);
			followBlock->dom = joinBlock;
			ssa->updateNop(elseHead);

			SSAValue* ssaAtIndexInBody = whileBodyHead;
			std::unordered_map<SSAValue*, SSAValue*> instReplacements;

			while (ssaAtIndexInBody != whileBodyTail) {
				SSAValue* leftOp = ssaAtIndexInBody->operand1;
				SSAValue* rightOp = ssaAtIndexInBody->operand2;
;
				try {
					std::string leftName = ssaAtIndexInBody->op1Name;
					std::tuple<SSAValue*, SSAValue*> replaceInfo = idChanges.at(leftName);
					if (leftOp == std::get<0>(replaceInfo)) {
						ssaAtIndexInBody->operand1 = std::get<1>(replaceInfo);
					}
				}
				catch (std::out_of_range& oor) {
					// intentionally left blank
				}

				try {
					std::string rightName = ssaAtIndexInBody->op2Name;
					std::tuple<SSAValue*, SSAValue*> replaceInfo = idChanges.at(rightName);
					if (rightOp == std::get<0>(replaceInfo)) {
						ssaAtIndexInBody->operand2 = std::get<1>(replaceInfo);
					}
				}
				catch (std::out_of_range& oor) {
					// intentionally left blank
				}

				try {
					SSAValue* replaceWith = instReplacements.at(ssaAtIndexInBody->operand1);
					ssaAtIndexInBody->operand1 = replaceWith;
				} catch (std::out_of_range& oor) {
					// intentionally left blank
				}
				try {
					SSAValue* replaceWith = instReplacements.at(ssaAtIndexInBody->operand2);
					ssaAtIndexInBody->operand2 = replaceWith;
				}
				catch (std::out_of_range& oor) {
					// intentionally left blank
				}
				SSAValue* iter = ssaAtIndexInBody->prevDomWithOpcode;
				bool needToEliminate = false;
				SSAValue* elimWith = nullptr;
				while (iter != nullptr && !needToEliminate) {
					if (iter->operand1 == ssaAtIndexInBody->operand1 && iter->operand2 == ssaAtIndexInBody->operand2) {
						// eliminate
						needToEliminate = true;
						elimWith = iter;
					}
					iter = iter->prevDomWithOpcode;
				}
				if (needToEliminate) {
					//SSAValue* beforeCurrent = ssaAtIndexInBody->prev;
					SSAValue* afterCurrent = ssaAtIndexInBody->next;
					//beforeCurrent->next = afterCurrent;
					//afterCurrent->prev = beforeCurrent;
					ssaAtIndexInBody->eliminated = true;

					SSAValue* elimIter = elimWith;
					while (elimIter->eliminatedBy != nullptr) {
						elimIter = elimIter->eliminatedBy;
					}

					instReplacements.insert({ ssaAtIndexInBody, elimIter });
					ssaAtIndexInBody->eliminatedBy = elimIter;
					ssaAtIndexInBody = afterCurrent;
				} else {
					ssaAtIndexInBody = ssaAtIndexInBody->next;
				}

			}
			// now rename operands of phis if needed

			SSAValue* iter = joinBlockHead->next;
			while (iter != joinBlockTail->next) {
				std::cout << "join block tail " << joinBlockTail->instCFGRepr() << std::endl;
				std::cout << "renaming while phis " << iter->instCFGRepr() << std::endl;
				try {
					SSAValue* replaceWith = instReplacements.at(iter->operand1);
					iter->operand1 = replaceWith;
				}
				catch (std::out_of_range& oor) {
					// intentionally left blank
				}
				try {
					SSAValue* replaceWith = instReplacements.at(iter->operand2);
					iter->operand2 = replaceWith;
				}
				catch (std::out_of_range& oor) {
					// intentionally left blank
				}
				iter = iter->next;
			}



			if (sym == OD) {
				printItem(getTextForEnum(sym));
				next();
			} else {
				error("While statement must end in 'od'");
			}
		} else {
			error("While statement must have 'do'");
		}
	} else {
		error("While statement must have 'while'");
	}

	if (savedJoinBlock != nullptr && savedJoinBlock->joinType != "while") {

		ssa->connectFT(followBlock, savedJoinBlock);
	}
	ssa->setInWhile(savedInWhile);
	elseHead = savedElseHead;
	joinBlockHead = savedJoinBlockHead;
	phiMap = savedPhiMap;
	joinBlock = savedJoinBlock;

	decPrintInd();

}

void Parser::returnStatement() {
	startPrintBlock("Return Statement");
	if (sym == RETURN) {
		printItem(getTextForEnum(sym));
		next();
		if ((sym == IDENT) || (sym == NUMBER) ||
			(sym == L_PAREN) || (sym == CALL)) {
			std::tuple<SSAValue*, std::string> tup = expression();
			SSAValue* left = std::get<0>(tup);
			ssa->SSACreate(ret, left);
		}

	} else {
		error("Return statement must have 'return'");
	}
	decPrintInd();
}

void Parser::statement() {
	startPrintBlock("Statement");
	if (sym == LET) {
		assignment();
	} else if (sym == CALL) {
		funcCall();
	} else if (sym == IF) {
		ifStatement();
	} else if (sym == WHILE) {
		whileStatement();
	} else if (sym == RETURN) {
		returnStatement();
	} else {
		std::cout << "error symbol" << getTextForEnum(sym) << std::endl;
		error("Statement type doesn't exist");
	}
	decPrintInd();
}

void Parser::statSequence() {
	startPrintBlock("Statement Sequence");

	ssa->enterScope();
	statement();
	while (sym == SEMICOLON) {
		printItem(getTextForEnum(sym));
		next(); // consume semicolon
		if ((sym == LET) || (sym == CALL) || (sym == IF) ||
			(sym == WHILE) || (sym == RETURN)) {
			statement();

		}
	}
	phiMap = ssa->exitScope();


	decPrintInd();
}

void Parser::varDecl() {
	startPrintBlock("Variable Declaration");
	if (sym == VAR) {
		printItem(getTextForEnum(sym));
		next();
		if (sym == IDENT) {
			ssa->addToVarDecl(getCurrentValue());
			printItem(getTextForEnum(sym), getCurrentValue());

			next();
			while (sym == COMMA) {
				printItem(getTextForEnum(sym));
				next();
				if (sym == IDENT) {
					ssa->addToVarDecl(getCurrentValue());
					printItem(getTextForEnum(sym), getCurrentValue());
					next();
				} else {
					error("Must have identifier in Var Declaration list");
				}
			}
			if (sym == SEMICOLON) {
				printItem(getTextForEnum(sym));
				next();
			} else {
				error("Var Declaration list must end with ';'");
			}
		} else {
			error("Var Declaration must have at least one identifier");
		}
	} else {
		error("Var Declaration must start with 'var'");
	}
	decPrintInd();
}


void Parser::funcDecl() {

	startPrintBlock("Function Declaration");
	bool isVoid = false;
	if (sym == VOID) {
		isVoid = true;
		printItem(getTextForEnum(sym));
		next();
	}
	if (sym == FUNCTION) {
		printItem(getTextForEnum(sym));
		next();
		if (sym == IDENT) {
			std::string ident = getCurrentValue();
			printItem(getTextForEnum(sym), getCurrentValue());
			ssa = new SSA(ident);
			programSSAs.insert({ ident, ssa });
			next();
			std::vector<std::string> params = formalParam();
			funcDescriptor newFunc = { isVoid, params };
			funcDescriptors.insert({ ident, newFunc });


			std::cout << "before next() " << std::endl;
			printItem(getTextForEnum(sym));
			printItem(getTextForEnum(sym));

			if (sym == SEMICOLON) {
				printItem(getTextForEnum(sym));
				next();
				funcBody();
				if (sym == SEMICOLON) {
					printItem(getTextForEnum(sym));
					next();
				} else {
					error("Function Body must be followed by ';'");
				}
			} else {
				error("Formal Params must be followed by ';'");
			}
		} else {
			error("Function declaration must have identifier");
		}
	}
	decPrintInd();
	ssa = programSSAs.at("__main__");
}

std::vector<std::string> Parser::formalParam() {
	startPrintBlock("Formal Parameters");
	std::cout << "sym is in fp " << getTextForEnum(sym) << std::endl;
	std::vector<std::string> formalParams;
	if (sym == L_PAREN) {
		printItem(getTextForEnum(sym));
		next();
		if (sym == R_PAREN) {
			printItem(getTextForEnum(sym));
			next();
		} else {
			if (sym == IDENT) {
				printItem(getTextForEnum(sym), getCurrentValue());
				SSAValue* argVal = ssa->SSACreateArgAssign(getCurrentValue());
				ssa->addToVarDecl(getCurrentValue());
				ssa->addSymbol(getCurrentValue(), argVal);
				formalParams.push_back(getCurrentValue());
				next();
				while (sym == COMMA) {
					printItem(getTextForEnum(sym));
					next(); // consume COMMA
					if (sym == IDENT) {
						printItem(getTextForEnum(sym), getCurrentValue());
						SSAValue* argVal = ssa->SSACreateArgAssign(getCurrentValue());
						ssa->addToVarDecl(getCurrentValue());
						ssa->addSymbol(getCurrentValue(), argVal);
						formalParams.push_back(getCurrentValue());
						next(); // consume IDENT
					} else {
						error("Expected identifier here");
					}

				}
				if (sym == R_PAREN) {
					printItem(getTextForEnum(sym));
					next();
				} else {
					error("Mismatched Parentheses");
				}
			}
		}

	} else {
		error("Formal params must begin with '('");
	}
	decPrintInd();
	return formalParams;
}

void Parser::funcBody() {
	startPrintBlock("Function Body");
	while (sym == VAR) {
		varDecl();
	}
	if (sym == L_BRACE) {
		printItem(getTextForEnum(sym));
		next();
		if ((sym == LET) || (sym == CALL) || (sym == IF) ||
			(sym == WHILE) || (sym == RETURN)) {
			statSequence();
		}
		if (sym == R_BRACE) {
			printItem(getTextForEnum(sym));
			next();
		} else {
			error("Function Body must end with '}'");
		}
	} else {
		error("Function Body must have '{' after var declaration list");
	}
	decPrintInd();
}

void Parser::computation() {
	if (debug) {
		std::cout << "Computation" << std::endl;
	}

	if (sym == MAIN) {
		printItem(getTextForEnum(sym));
		next();
		while (sym == VAR) {
			varDecl();
		}
		while ((sym == VOID) || (sym == FUNCTION)) {
			funcDecl();
		}

		if (sym == L_BRACE) {
			printItem(getTextForEnum(sym));
			next();
			statSequence();
			if (sym == R_BRACE) {
				printItem(getTextForEnum(sym));
				next();
				if (sym == PERIOD) {
					printItem(getTextForEnum(sym));
					next();
				} else {
					error("Computation must end with '.'");
				}

			} else {
				error("Computation statement sequence must end with '}'");
			}
		} else {
			error("Computation must have '{' after function declarations");
		}
	} else {
		error("Computation must have 'main'");
	}
}


void Parser::parse() {
	if (debug) {
		std::cout << "Parser is starting..." << std::endl;
	}
	ssa = new SSA("__main__");
	programSSAs.insert({ "__main__", ssa });
	funcDescriptor read = { false, std::vector<std::string>() };
	std::vector<std::string> writeParams;
	writeParams.push_back("toWrite");
	funcDescriptor write = { true, writeParams };
	funcDescriptor writeNL = { true, std::vector<std::string>() };

	funcDescriptors.insert({ "InputNum", read});
	funcDescriptors.insert({ "OutputNum", write });
	funcDescriptors.insert({ "OutputNewLine", writeNL });

	currentPrintIndent = 0;
	currPos = 0;
	sym = tokens[currPos].getType();

	computation();
}

std::string Parser::outputSSA() {
	return ssa->outputSSA();
}

void Parser::reset() {
	ssa->reset();
}
void Parser::printSSA() {
	std::cout << "All program SSAs" << std::endl;
	for (auto kv : programSSAs) {
		ssa = kv.second;
		std::cout << "// " << kv.first << ": " << std::endl;
		std::cout << std::endl;
		ssa->printSSA();
		std::cout << std::endl;
		ssa->printSymTable();
		std::cout << std::endl;
		ssa->printConstTable();
		std::cout << std::endl;
		ssa->printVarDeclList();
		std::cout << std::endl;

	}




}


void Parser::printDotLang() {
	//ssa->generateDotLang();
	std::cout << "digraph prog {" << std::endl;
	std::cout << "program[label = \"Program\"];" << std::endl;
	for (auto kv : programSSAs) {
		ssa = kv.second;
		ssa->gen();
		std::cout << "program:s -> " + kv.first + "CONST:n [label=\"\"]" << std::endl;

	}
	std::cout << "}" << std::endl;
}
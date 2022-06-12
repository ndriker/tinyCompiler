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
	incPrintInd();
	std::cout << createIndent() << item << " " << value << std::endl;
	decPrintInd();
}

void Parser::startPrintBlock(std::string blockName) {
	incPrintInd();
	std::cout << createIndent() << blockName << std::endl;
}

SSAValue* Parser::varRef() {
	startPrintBlock("Var Ref");

	std::string ident = getCurrentValue();
	printItem("Identifier", ident);
	SSAValue* identInst = ssa.findSymbol(ident);

	next(); // consume ident
	decPrintInd();
	return identInst;
}

SSAValue* Parser::factor() {
	startPrintBlock("Factor");

	SSAValue* left;

	if (sym == IDENT) {
		left = varRef();
	}
	else if (sym == NUMBER) {
		std::string numStr = getCurrentValue();
		printItem("Number", numStr);
		int num = stoi(numStr);
		left = ssa.SSACreateConst(num);
		next();
	}
	else if (sym == L_PAREN) {
		printItem("Left Paren");
		next();
		left = expression();

		if (sym == R_PAREN) {
			printItem("Right Paren");
			next();
		}
		else {
			error("Mismatched Parentheses");
		}
	}
	else if (sym == CALL) {
		funcCall();
		// TODO
		left = new SSAValue();
	}
	else {
		error("No Production Exists for your input");
		// TODO
		left = new SSAValue();
	}
	
	decPrintInd();
	return left;
}

SSAValue* Parser::term() {
	startPrintBlock("Term");
	bool wasMul;

	SSAValue* left;
	SSAValue* right;

	left = factor();
	while ((sym == MUL) || (sym == DIV)) {
		wasMul = (sym == MUL) ? true : false;
		printItem(getTextForEnum(sym));
		next();
		right = factor();
		if (wasMul) {
			left = ssa.SSACreate(MULOP, left, right);
		} else {
			// sym == div
			left = ssa.SSACreate(DIVOP, left, right);
		}
	}

	decPrintInd();
	return left;
}

SSAValue* Parser::expression() {
	startPrintBlock("Expression");
	bool wasAdd;
	SSAValue* left;
	SSAValue* right;

	left = term();
	while ((sym == ADD) || (sym == SUB)) {
		wasAdd = (sym == ADD) ? true : false;
		printItem(getTextForEnum(sym));
		next();
		right = term();
		if (wasAdd) {
			left = ssa.SSACreate(ADDOP, left, right);
		} else {
			// sym == sub
			left = ssa.SSACreate(SUBOP, left, right);
		}
	}
	decPrintInd();
	return left;
}

SSAValue* Parser::relation() {
	startPrintBlock("Relation");
	SSAValue* left;
	SSAValue* right = nullptr;
	tokenType symCopy;
	left = expression();
	if ((sym >= 18) || (sym <= 23)) {
		symCopy = sym;
		printItem(getTextForEnum(sym));

		// take sym to know which relational op it is
		next();
		right = expression();
	}
	SSAValue* cmp = ssa.SSACreate(CMP, left, right);
	elseHead = ssa.SSACreateNop();
	SSAValue* rel = ssa.SSACreate(ssa.convertBr(symCopy), cmp, elseHead);
	decPrintInd();
	return rel;
}

void Parser::assignment() {
	startPrintBlock("Assignment");
	std::string identName;
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
				left = expression();
				ssa.addSymbol(identName, left);

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

void Parser::funcCall() {
	startPrintBlock("Function Call");
	if (sym == CALL) {
		printItem(getTextForEnum(sym));
		next();
		if (sym == IDENT) {
			printItem(getTextForEnum(sym), getCurrentValue());
			std::string ident = getCurrentValue();
			if (ident == "InputNum") {
				ssa.SSACreate(read, nullptr, nullptr);
			}
			next();
			

			if (sym == L_PAREN) {
				// do the with paren version
				printItem(getTextForEnum(sym));
				next(); // consume left paren
				if (sym == R_PAREN) {
					printItem(getTextForEnum(sym));
					next(); // consume right paren
				} else {
					SSAValue* result = expression();
					if (ident == "OutputNum") {
						ssa.SSACreate(write, result);
					}
					while (sym == COMMA) {
						printItem(getTextForEnum(sym));
						next(); // consume comma
						expression();

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


	if (sym == IF) {
		printItem(getTextForEnum(sym));
		next();
		relation();
		if (sym == THEN) {
			printItem(getTextForEnum(sym));
			next();

			phiMap = std::unordered_map<std::string, SSAValue*>();
			statSequence();
			std::unordered_map<std::string, SSAValue*> thenPhiMap = phiMap;

			if (sym == ELSE) {
				joinBlockHead = ssa.SSACreateNop();
				ssa.SSACreate(BRA, joinBlockHead);
				ssa.updateNop(elseHead);
				printItem(getTextForEnum(sym));
				next();

				phiMap = std::unordered_map<std::string, SSAValue*>();
				
				statSequence();
				
				std::unordered_map<std::string, SSAValue*> elsePhiMap = phiMap;
				
				ssa.updateNop(joinBlockHead);

				for (auto kv : thenPhiMap) {
					try {
						SSAValue* val = elsePhiMap.at(kv.first);
						ssa.SSACreate(PHI, kv.second, val);
						elsePhiMap.erase(kv.first);
						ssa.addSymbol(kv.first, ssa.getTail());
					}
					catch (std::out_of_range& oor) {
						SSAValue* prevOccur = ssa.findSymbol(kv.first);
						ssa.SSACreate(PHI, kv.second, prevOccur);
						ssa.addSymbol(kv.first, ssa.getTail());

					}
				}
				for (auto kv : elsePhiMap) {
					SSAValue* prevOccur = ssa.findSymbol(kv.first);
					ssa.SSACreate(PHI, prevOccur, kv.second);
					ssa.addSymbol(kv.first, ssa.getTail());

				}
			} else {
				// if there is no else, elseHead becomes 
				//		the head of the join block, no branch needed
				//		just fall through
				ssa.updateNop(elseHead);
				for (auto kv : thenPhiMap) {
					SSAValue* prevOccur = ssa.findSymbol(kv.first);
					ssa.SSACreate(PHI, kv.second, prevOccur);
					ssa.addSymbol(kv.first, ssa.getTail());

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


	elseHead = savedElseHead;
	joinBlockHead = savedJoinBlockHead;
	phiMap = savedPhiMap;
	decPrintInd();
}

void Parser::whileStatement() {
	startPrintBlock("While Statement");

	SSAValue* savedElseHead = elseHead;
	SSAValue* savedJoinBlockHead = joinBlockHead;
	std::unordered_map<std::string, SSAValue*> savedPhiMap = phiMap;

	if (sym == WHILE) {
		printItem(getTextForEnum(sym));
		next();
		SSAValue* branchInst = relation();

		joinBlockHead = ssa.SSACreateNop();

		if (sym == DO) {
			printItem(getTextForEnum(sym));
			next();
			phiMap = std::unordered_map<std::string, SSAValue*>();
			SSAValue* whileBodyHead = ssa.SSACreateNop();
			ssa.updateNop(whileBodyHead);
			statSequence();
			ssa.SSACreate(BRA, joinBlockHead);
			SSAValue* whileBodyTail = ssa.getTail();

			std::unordered_map<std::string, SSAValue*> whilePhiMap = phiMap;

			ssa.updateNop(joinBlockHead);
			SSAValue* joinBlockTail = joinBlockHead;

			std::unordered_map<SSAValue*, SSAValue*> idChanges;
			for (auto kv : whilePhiMap) {
				SSAValue* prevOccur = ssa.findSymbol(kv.first);
				SSAValue* phiInst = ssa.SSACreate(PHI, prevOccur, kv.second);
				idChanges.insert_or_assign(prevOccur, phiInst);
				joinBlockTail = phiInst;
				ssa.addSymbol(kv.first, ssa.getTail());
			}

			SSAValue* aboveCmpInst = branchInst->prev->prev;


			SSAValue* cmpInst = branchInst->prev;
			try {
				cmpInst->operand1 = idChanges.at(cmpInst->operand1);
			} catch (std::out_of_range& oor) {
				std::cout << "Most likely infinite loop, check that you are modifying the loop control variable." << std::endl;
				throw std::logic_error("Most likely infinite loop, check that you are modifying the loop control variable.");
			}


			aboveCmpInst->next = joinBlockHead;
			joinBlockHead->prev = aboveCmpInst;



			joinBlockTail->next = cmpInst;
			cmpInst->prev = joinBlockTail;

			branchInst->next = whileBodyHead;
			whileBodyHead->prev = branchInst;

			ssa.setInstTail(whileBodyTail);

			ssa.updateNop(elseHead);

			SSAValue* ssaAtIndexInBody = whileBodyHead;
			while (ssaAtIndexInBody != whileBodyTail) {
				try {
					SSAValue* renameLeftInst = idChanges.at(ssaAtIndexInBody->operand1);
					ssaAtIndexInBody->operand1 = renameLeftInst;

					//SSAValue* renameRightInst = idChanges.at(ssaAtIndexInBody->operand2);
					//ssaAtIndexInBody->operand2 = renameRightInst;
				} catch (std::out_of_range& oor) {

				}
				ssaAtIndexInBody = ssaAtIndexInBody->next;
			}
			//aboveCmpInst->instRepr();
			//aboveCmpInst->next->instRepr();
			//aboveCmpInst->next->next->instRepr();
			//aboveCmpInst->next->next->next->instRepr();
			//aboveCmpInst->next->next->next->next->instRepr();
			//aboveCmpInst->next->next->next->next->next->instRepr();
			//aboveCmpInst->next->next->next->next->next->next->instRepr();
			//aboveCmpInst->next->next->next->next->next->next->next->instRepr();
			//aboveCmpInst->next->next->next->next->next->next->next->next->instRepr();
			//aboveCmpInst->next->next->next->next->next->next->next->next->next->instRepr();



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



	elseHead = savedElseHead;
	joinBlockHead = savedJoinBlockHead;
	phiMap = savedPhiMap;

	decPrintInd();

}

void Parser::returnStatement() {
	startPrintBlock("Return Statement");
	if (sym == RETURN) {
		printItem(getTextForEnum(sym));
		next();
		if ((sym == IDENT) || (sym == NUMBER) ||
			(sym == L_PAREN) || (sym == CALL)) {
			expression();
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

	ssa.enterScope();
	statement();
	while (sym == SEMICOLON) {
		printItem(getTextForEnum(sym));
		next(); // consume semicolon
		if ((sym == LET) || (sym == CALL) || (sym == IF) ||
			(sym == WHILE) || (sym == RETURN)) {
			statement();

		}
	}
	phiMap = ssa.exitScope();


	decPrintInd();
}

void Parser::varDecl() {
	startPrintBlock("Variable Declaration");
	if (sym == VAR) {
		printItem(getTextForEnum(sym));
		next();
		if (sym == IDENT) {
			printItem(getTextForEnum(sym), getCurrentValue());
			next();
			while (sym == COMMA) {
				printItem(getTextForEnum(sym));
				next();
				if (sym == IDENT) {
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
	if (sym == VOID) {
		printItem(getTextForEnum(sym));
		next();
	}
	if (sym == FUNCTION) {
		printItem(getTextForEnum(sym));
		next();
		if (sym == IDENT) {
			printItem(getTextForEnum(sym), getCurrentValue());
			next();
			formalParam();
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
}

void Parser::formalParam() {
	startPrintBlock("Formal Parameters");
	if (sym == L_PAREN) {
		printItem(getTextForEnum(sym));
		next();
		if (sym == R_PAREN) {
			printItem(getTextForEnum(sym));
			next();
		} else {
			if (sym == IDENT) {
				printItem(getTextForEnum(sym), getCurrentValue());
				next();
				while (sym == COMMA) {
					printItem(getTextForEnum(sym));
					next(); // consume COMMA
					if (sym == IDENT) {
						printItem(getTextForEnum(sym), getCurrentValue());
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
	std::cout << "Computation" << std::endl;
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
	std::cout << "Parser is starting..." << std::endl;
	currentPrintIndent = 0;
	currPos = 0;
	sym = tokens[currPos].getType();

	computation();
}

void Parser::printSSA() {
	std::cout << std::endl;
	ssa.printSSA();
	std::cout << std::endl;
	ssa.printSymTable();
	std::cout << std::endl;
	ssa.printConstTable();
	std::cout << std::endl;
}


void Parser::printDotLang() {
	ssa.generateDotLang();
}
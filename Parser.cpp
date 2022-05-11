#include "parser.h"
#include <iostream>
#include <vector>
#include "token.h"

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

void Parser::varRef() {
	startPrintBlock("Var Ref");
	printItem("Identifier", getCurrentValue());
	next(); // consume ident
	decPrintInd();
}

void Parser::factor() {
	startPrintBlock("Factor");

	if (sym == IDENT) {
		varRef();
	}
	else if (sym == NUMBER) {
		printItem("Number", getCurrentValue());
		next();
	}
	else if (sym == L_PAREN) {
		printItem("Left Paren");
		next();
		expression();

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
	}
	else {
		error("No Production Exists for your input");
	}
	
	decPrintInd();
}

void Parser::term() {
	startPrintBlock("Term");
	factor();
	while ((sym == MUL) || (sym == DIV)) {
		printItem(getTextForEnum(sym));
		next();
		factor();
	}
	decPrintInd();
}

//void Parser::relOp() {
//	if ()
//}
void Parser::expression() {
	startPrintBlock("Expression");
	term();
	while ((sym == ADD) || (sym == SUB)) {
		printItem(getTextForEnum(sym));
		next();
		term();
	}
	decPrintInd();
}

void Parser::relation() {
	startPrintBlock("Relation");
	expression();
	if ((sym >= 18) || (sym <= 23)) {
		printItem(getTextForEnum(sym));

		// take sym to know which relational op it is
		next();
		expression();
	}
	decPrintInd();
}

void Parser::assignment() {
	startPrintBlock("Assignment");
	if (sym == LET) {
		printItem(getTextForEnum(sym));
		next(); // consume LET
		if (sym == IDENT) {
			printItem(getTextForEnum(sym), getCurrentValue());

			next(); // consume IDENT

			if (sym == ASSIGN) {
				printItem(getTextForEnum(sym));

				next(); // consume ASSIGN
				expression();

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
			next();

			if (sym == L_PAREN) {
				// do the with paren version
				printItem(getTextForEnum(sym));
				next(); // consume left paren
				if (sym == R_PAREN) {
					printItem(getTextForEnum(sym));
					next(); // consume right paren
				} else {
					expression();
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
	if (sym == IF) {
		printItem(getTextForEnum(sym));
		next();
		relation();
		if (sym == THEN) {
			printItem(getTextForEnum(sym));
			next();
			statSequence();
			
			if (sym == ELSE) {
				printItem(getTextForEnum(sym));
				next();
				statSequence();
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
	decPrintInd();
}

void Parser::whileStatement() {
	startPrintBlock("While Statement");
	if (sym == WHILE) {
		printItem(getTextForEnum(sym));
		next();
		relation();
		if (sym == DO) {
			printItem(getTextForEnum(sym));
			next();
			statSequence();
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
	statement();
	while (sym == SEMICOLON) {
		printItem(getTextForEnum(sym));
		next(); // consume semicolon
		if ((sym == LET) || (sym == CALL) || (sym == IF) ||
			(sym == WHILE) || (sym == RETURN)) {
			statement();

		}
	}

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
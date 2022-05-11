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

void Parser::error(std::string errorMessage) {
	std::cout << "Parser Error: " << errorMessage << std::endl;
}

std::string Parser::createIndent() {
	return std::string(3 * currentPrintIndent, ' ');
}


void Parser::varRef() {
	currentPrintIndent += 1;
	std::cout << createIndent() << "Ident" << std::endl;
	currentPrintIndent -= 1;
}

void Parser::factor() {

	if (sym == IDENT) {
		currentPrintIndent += 1;
		std::cout << createIndent() << "VarRef" << std::endl;
		varRef();
		currentPrintIndent -= 1;
		next();
	}
	else if (sym == NUMBER) {
		currentPrintIndent += 1;
		std::cout << createIndent() << "Number" << std::endl;
		currentPrintIndent -= 1;
		next();
	}
	else if (sym == L_PAREN) {
		next();
		currentPrintIndent += 1;
		std::cout << createIndent() << "Expression" << std::endl;
		expression();
		currentPrintIndent -= 1;

		if (sym == R_PAREN) {
			next();
		}
		else {
			error("Mismatched Parentheses");
		}
	}
	else if (sym == CALL) {
		currentPrintIndent += 1;
		std::cout << createIndent() << "Call" << std::endl;
		funcCall();
		currentPrintIndent -= 1;
		next();
	}
	else {
		error("No Production Exists for your input");
	}
}

void Parser::term() {
	currentPrintIndent += 1;
	std::cout << createIndent() << "Factor" << std::endl;
	factor();
	currentPrintIndent -= 1;
	while ((sym == MUL) || (sym == DIV)) {
		next();
		currentPrintIndent += 1;
		std::cout << createIndent() << "Factor" << std::endl;
		factor();
		currentPrintIndent -= 1;
	}
}

void Parser::relOp() {
	if ()
}
void Parser::expression() {

	currentPrintIndent += 1;
	std::cout << createIndent() << "Term" << std::endl;
	term();
	currentPrintIndent -= 1;
	while ((sym == ADD) || (sym == SUB)) {
		next();
		currentPrintIndent += 1;
		std::cout << createIndent() << "Term" << std::endl;
		term();
		currentPrintIndent -= 1;
	}
}

void Parser::relation() {
	currentPrintIndent += 1;
	expression();
	next();
	if ((sym >= 18) || (sym <= 23)) {
		// take sym to know which relational op it is
		expression();
	}
}




void Parser::parse() {
	std::cout << "Parser is starting..." << std::endl;
	currentPrintIndent = 0;
	currPos = 0;
	sym = tokens[currPos].getType();

	std::cout << "Expression" << std::endl;
	expression();
}
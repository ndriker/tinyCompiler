// tinyCompiler.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include "lexer.h"
#include "parser.h"


int main(int argc, char* argv[])
{
	std::cout << "Starting compilation...\n";
	std::string fName = "test01.txt";//argv[0];

	// Lexer
	Lexer lex;
	lex.setFile(fName);
	lex.tokenize();
	lex.displayTokens();

	//// Parser
	//Parser parser;
	//parser.setTokens(lex.getTokens());
	//parser.parse();
}

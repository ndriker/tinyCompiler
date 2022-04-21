#ifndef __LEXER_H__
#define __LEXER_H__

#include <string>
#include <fstream>
#include <vector>
#include "token.h"

class Lexer {

	public:
		void setFile(std::string fileName);
		const char* lookahead(std::string& line, int& currentIndex);
		void displayTokens();
		std::vector<Token> getTokens();
		void tokenize();

	private:
		std::ifstream infile;
		std::vector<Token> tokens;
		enum state {
			IDLE = 0,
			IN_NUMBER = 1,
			IN_IDENT = 2
		};

		bool isDigit(const char* ch);
		bool isLetter(const char* ch);
		bool isAlphanum(const char* ch);



};


#endif
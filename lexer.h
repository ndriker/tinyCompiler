#ifndef __LEXER_H__
#define __LEXER_H__

#include <string>
#include <fstream>
#include <vector>
#include <exception>
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
			IDLE		 = 0,
			IN_NUMBER	 = 1,
			IN_IDENT	 = 2,
			SUCC_NOT	 = 3,
			SUCC_LT		 = 4,
			SUCC_GT		 = 5,
			SUCC_EQ		 = 6,
			IN_COMMENT   = 7,
			EXIT_COMMENT = 8
		};
		bool isDigit(const char* ch);
		bool isLetter(const char* ch);
		bool isAlphanum(const char* ch);
		bool isRelationalOpStem(const char* ch);
		int isKeyword(std::string accumulator);


};

class LexerError : virtual public std::exception {
	protected:
		std::string error_message;
		int lineNumber;
		int columnNumber;
		std::string errMsg;


	public:
		explicit
		LexerError(const std::string& msg, int line_num, int column_num);
	
		const char* what() const throw ();

};


#endif
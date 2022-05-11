#ifndef __PARSER__
#define __PARSER__

#include <vector>
#include "token.h"
//#include "SSAValue.h"

class Parser {
	public:
	//SSAValue Expression();
	//SSAValue 

		void setTokens(std::vector<Token> inTokens);

		void varRef();
		void factor();
		void term();
		void expression();
		void relation();

		void assignment();
		void funcCall();
		void ifStatement();
		void whileStatement();
		void returnStatement();

		void statement();
		void statSequence();

		void varDecl();
		void funcDecl();
		void formalParam();
		void funcBody();
		void computation();



		void parse();
	private:
		std::vector<Token> tokens;
		int currPos;
		tokenType sym;
		int currentPrintIndent;
		void next();
		std::string getCurrentValue();

		void error(std::string errorMessage);

		std::string createIndent();
		void incPrintInd();
		void decPrintInd();
		void printItem(std::string item, std::string value = "");
		void startPrintBlock(std::string blockName);

};

#endif

#ifndef __PARSER_H__
#define __PARSER_H__

#include <vector>
#include "token.h"
#include "ssa.h"



class Parser {
	public:
	//SSAValue Expression();
	//SSAValue 

		void setTokens(std::vector<Token> inTokens);
		void setDebug(bool debugMode);

		SSAValue* varRef();
		std::tuple<SSAValue*, std::string> factor();
		std::tuple<SSAValue*, std::string> term();
		std::tuple<SSAValue*, std::string> expression();
		SSAValue* relation();

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
		std::string outputSSA();
		void printSSA();
		void printDotLang();

		void reset();

	private:
		bool debug;
		std::vector<Token> tokens;
		int currPos;
		tokenType sym;
		int currentPrintIndent;
		SSA ssa;

		SSAValue* elseHead;
		SSAValue* joinBlockHead;
		BasicBlock* joinBlock;

		std::unordered_map<std::string, SSAValue*> phiMap;


		// traversal functions
		void next();
		std::string getCurrentValue();

	
		void error(std::string errorMessage);





		// functions for printing parse tree
		std::string createIndent();
		void incPrintInd();
		void decPrintInd();
		void printItem(std::string item, std::string value = "");
		void startPrintBlock(std::string blockName);

};

#endif

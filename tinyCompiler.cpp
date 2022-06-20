// tinyCompiler.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include "lexer.h"
#include "parser.h"


#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

// trim from start
static inline std::string& ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),
		std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

// trim from end
static inline std::string& rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(),
		std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

// trim from both ends
static inline std::string& trim(std::string& s) {
	return ltrim(rtrim(s));
}

std::string compile(std::string fName, bool testMode = false) {
	if (!testMode) {

		// Lexer
		Lexer lex;
		lex.setFile(fName);
		lex.tokenize();
		lex.displayTokens();

		// Parser
		Parser parser;
		parser.setTokens(lex.getTokens());
		parser.setDebug(true);
		parser.parse();
		parser.printSSA();
		parser.printDotLang();
		parser.reset();
		return "";
	} else {
		// Lexer
		Lexer lex;
		lex.setFile(fName);
		lex.tokenize();

		// Parser
		Parser parser;
		parser.setTokens(lex.getTokens());
		parser.setDebug(false);
		parser.parse();

		std::string res = parser.outputSSA();
		parser.reset();
		return res;
	}
}


bool confirmResults(std::string baseFileName) {
	std::string testFile = baseFileName + "_test.tiny";
	std::string expectedResFile = baseFileName + "_expected.txt";
	
	std::string results = compile(testFile, true);
	// perform diff on results and expectedResFile
	std::ifstream expected(expectedResFile);
	std::stringstream expectedBuffer;
	expectedBuffer << expected.rdbuf();
	if (results.compare(expectedBuffer.str()) == 0) {
		return true;
	}

	// return true if correct otherwise return false
	return false;
}

std::tuple<bool, std::string> testCompiler() {
	// open index file
	// iterate through index file and compile test file
	// confirm results of test file and expected res file
	// return true if all tests passed, and summary of how many tests passed/ failed

	std::ifstream infile("./index.txt");

	std::string line;

	bool allPassed = true;
	int totalNumberTests = 0;
	int totalNumberPassed = 0;
	std::vector<std::string> testsFailed;

	while (std::getline(infile, line)) {
		totalNumberTests += 1;
		std::string testFileNameDelimiter = ":";


		int colonDelimInd = line.find(testFileNameDelimiter);
		std::string testFileName = line.substr(0, colonDelimInd - 1);
		testFileName = trim(testFileName);

		std::string description = line.substr(colonDelimInd + 1);
		description = trim(description);

		std::cout << "Testing " << line << std::endl;
		bool passed = confirmResults(testFileName);
		if (passed) {
			std::cout << "\t  PASSED" << std::endl;
			totalNumberPassed += 1;
		} else {
			std::cout << "\t  FAILED" << std::endl;
			testsFailed.push_back(testFileName);

		}
		allPassed = allPassed && passed;
		//std::cout << "Test #" << test_number << std::endl;
		//std::cout << testFileName << std::endl;
		//std::cout << description << std::endl;
	}
	std::string finalResult = allPassed ? "PASSED" : "FAILED";
	std::string summary = "Test Summary : " + finalResult + "\n" + 
		                   std::to_string(totalNumberPassed) + "/" + 
		                   std::to_string(totalNumberTests) + " passed.";
	if (totalNumberPassed != totalNumberTests) {
		summary += "\nThe following tests failed: ";
		for (int i = 0; i < testsFailed.size(); i++) {
			summary += testsFailed[i] + ", ";
		}
		summary = summary.substr(0, summary.size() - 2);
	}


	std::cout << summary << std::endl;

	std::tuple<bool, std::string> info = std::make_tuple(allPassed, summary);
	return info;
}

int main(int argc, char* argv[]) {
	std::cout << "Starting compilation...\n";
	std::string fName = "tests/testSSA.tiny";//argv[0];

	std::string output = compile("./tests/testSSA.tiny");
	//testCompiler();

	return 0;

}


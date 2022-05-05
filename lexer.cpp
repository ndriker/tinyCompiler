#include "lexer.h"
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>
#include <vector>

#include "token.h"

void Lexer::setFile(std::string fileName) {
	infile.open(fileName);
}

bool Lexer::isDigit(const char* ch) {
	std::regex reg("[0-9]");
	return std::regex_match(ch, reg);
}

bool Lexer::isLetter(const char* ch) {
	std::regex reg("[a-zA-Z]");
	return std::regex_match(ch, reg);
}

bool Lexer::isAlphanum(const char* ch) {
	std::regex reg("[a-zA-Z0-9]");
	return std::regex_match(ch, reg);
}

bool Lexer::isRelationalOpStem(const char* ch) {
	std::regex reg("[!<>=]");
	return std::regex_match(ch, reg);
}

bool Lexer::isKeyword(std::string accumulator) {
	std::string keywords[14] = {
		"let",
		"var",
		"if",
		"then",
		"else",
		"fi",
		"while",
		"do",
		"od",
		"void",
		"function",
		"call",
		"return",
		"main"
	};
	for(int i = 0; i < 14; ++i) {
		if(accumulator.compare(keywords[i]) == 0) {
			return true;
		}
	}
	return false;
}



void Lexer::displayTokens() {
	std::cout << "Total tokens found: " << tokens.size() << std::endl;
	std::cout << "ID    Type        Ln[Cn]     Value" << std::endl;
	for (int i = 0; i < tokens.size(); i++) {
		tokens[i].displayToken(i);
	}
}

std::vector<Token> Lexer::getTokens() {
	return tokens;
}


void Lexer::tokenize() {
	std::cout << "Lexer is starting..." << std::endl;
	if (infile.is_open()) {
		std::string line;
		int lineNumber = 0;
		while (std::getline(infile, line)) {
			lineNumber += 1;
			std::string accumulator = "";
			int columnNumber = 0;
			int accumStartIdx = 0;

			state st = IDLE;

			for (int i = 0; i < line.size(); i++) {
				columnNumber += 1;
				char el = line[i];

				std::string str(1, el);
				const char* ch = str.c_str();

				switch (st) {
					case IDLE: {
						if (isDigit(ch)) {
							st = IN_NUMBER;
							std::string digit(1, el);
							accumulator.append(digit);
							accumStartIdx = columnNumber;

							// lookahead
							char nextEl = line[i + 1];
							std::string nextStr(1, nextEl);
							const char* nextCh = nextStr.c_str();

							if (!isDigit(nextCh)) {
								Token* number = new Token(NUMBER, lineNumber, accumStartIdx, accumulator);
								tokens.push_back(*number);
								st = IDLE;
								accumulator = "";
							}
						} else if (isLetter(ch)) {
							st = IN_IDENT;
							std::string letter(1, el);
							accumulator.append(letter);
							accumStartIdx = columnNumber;

							// lookahead
							char nextEl = line[i + 1];
							std::string nextStr(1, nextEl);
							const char* nextCh = nextStr.c_str();

							if (!isLetter(nextCh)) {
								// TODO add keyword checks here!!
								
								//if (isKeyword(accumulator)) {
								//	create keyword
								//} else {
								//	create ident
								//}
								Token* ident = new Token(IDENT, lineNumber, accumStartIdx, accumulator);
								tokens.push_back(*ident);
								st = IDLE;
								accumulator = "";
							}
						}
						else if (el == '!') {
							st = SUCC_NOT;
							std::string not_sym(1, el);
							accumulator.append(not_sym);
							accumStartIdx = columnNumber;

							// lookahead
							char nextEl = line[i + 1];
							std::string nextStr(1, nextEl);
							const char* nextCh = nextStr.c_str();

							if (nextCh != "=") {
								// lexer error
								st = IDLE;
								accumulator = "";
								throw(LexerError("Symbol '!' must be followed by Symbol '='", lineNumber, columnNumber));
							}
						} else if (el == '<') {
							// < is used for both relational op and for assignment
							// needs special treatment
							st = SUCC_LT;
							std::string lt(1, el);
							accumulator.append(lt);
							accumStartIdx = columnNumber;

							// lookahead
							char nextEl = line[i + 1];
							std::string nextStr(1, nextEl);
							const char* nextCh = nextStr.c_str();

							if ((nextCh != "-") && (nextCh != "=")) {
								Token* lessThan = new Token(LT, lineNumber, accumStartIdx, accumulator);
								tokens.push_back(*lessThan);
								st = IDLE;
								accumulator = "";	
								// possibly rename to aggregator
							}

						} else if (el == '>') {
							st = SUCC_GT;
							std::string gt(1, el);
							accumulator.append(gt);
							accumStartIdx = columnNumber;

							// lookahead
							char nextEl = line[i + 1];
							std::string nextStr(1, nextEl);
							const char* nextCh = nextStr.c_str();

							if (nextCh != "=") {
								Token* greaterThan = new Token(GT, lineNumber, accumStartIdx, accumulator);
								tokens.push_back(*greaterThan);
								st = IDLE;
								accumulator = "";
							}
						} else if (el == '=') {
							st = SUCC_EQ;
							std::string eq(1, el);
							accumulator.append(eq);
							accumStartIdx = columnNumber;

							// lookahead
							char nextEl = line[i + 1];
							std::string nextStr(1, nextEl);
							const char* nextCh = nextStr.c_str();

							if (nextCh != "=") {
								throw(LexerError("Symbol '=' must be followed by Symbol '='", lineNumber, columnNumber));
							}
						} else {
							switch (el) {
								// punctuation cases
								case '(': {
									Token* lparen = new Token(L_PAREN, lineNumber, columnNumber, "(");
									tokens.push_back(*lparen);
								} break;
								case ')': {
									Token* rparen = new Token(R_PAREN, lineNumber, columnNumber, ")");
									tokens.push_back(*rparen);
								} break;
								case '{':
								{
									Token* lbrace = new Token(L_BRACE, lineNumber, columnNumber, "{");
									tokens.push_back(*lbrace);
								} break;
								case '}': {
									Token* rbrace = new Token(R_BRACE, lineNumber, columnNumber, "}");
									tokens.push_back(*rbrace);
								} break;
								case ',': {
									Token* comma = new Token(COMMA, lineNumber, columnNumber, ",");
									tokens.push_back(*comma);
								} break;
								case ';': {
									Token* semicolon = new Token(SEMICOLON, lineNumber, columnNumber, ";");
									tokens.push_back(*semicolon);
								} break;
								case '.': {
									Token* period = new Token(PERIOD, lineNumber, columnNumber, ".");
									tokens.push_back(*period);
								} break;
							}
						}
					} break;
					case IN_NUMBER: {
						if (isDigit(ch)) {
							st = IN_NUMBER;
							std::string digit(1, el);
							accumulator.append(digit);
							i = i + 1;
							
						}
					} break;
				}
			}
		}
	}
}

// start LexerError implementations

LexerError::LexerError(const std::string & msg, int line_num, int column_num) {
	error_message = msg;
	lineNumber = line_num;
	columnNumber = column_num;
	std::string ln = std::to_string(lineNumber);
	std::string cn = std::to_string(columnNumber);
	errMsg = "Exception: " + msg + ", at " + ln + "[" + cn + "]";
}

const char* LexerError::what() const throw () {
	return errMsg.c_str();
}
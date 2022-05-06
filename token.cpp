#include "token.h"
#include <string>
#include <iostream>

Token::Token() {
	type = ADD;
	lineNumber = 0;
	columnNumber = 0;
	value = "";
}

Token::Token(tokenType tokenType, int ln, int cn, std::string val) {
	type = tokenType;
	lineNumber = ln;
	columnNumber = cn;
	value = val;
}

std::string Token::getTextForEnum(int enumVal) {
	return Token::tokenEnumStrings[enumVal];
}

void Token::displayToken(int id) {
	std::string tokenType = Token::getTextForEnum(type);
	std::string location = std::to_string(lineNumber) + "[" +
		std::to_string(columnNumber) + "]";

	printf("%-6d", id);
	printf("%-12s", tokenType.c_str());
	printf("%-11s", location.c_str());
	printf("%-11s", value.c_str());
	printf("%s", "\n");


}

tokenType Token::getType() {
	return type;
}
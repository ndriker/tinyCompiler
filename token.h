#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <string>

enum tokenType {
	LET			= 0, // let
	VAR			= 1, // var
	IF			= 2, // if
	THEN		= 3, // then
	ELSE		= 4, // else
	FI			= 5, // fi
	WHILE		= 6, // while
	DO			= 7, // do
	OD			= 8, // od
	VOID		= 9, // void
	FUNCTION	= 10,// function
	CALL		= 11,// call 
	RETURN		= 12,// return
	MAIN		= 13,// main
	ADD			= 14,// +
	SUB			= 15,// -
	MUL			= 16,// *
	DIV			= 17,// /
	EQ			= 18,// ==
	NEQ			= 19,// !=
	LT			= 20,// <
	LE			= 21,// <=
	GT			= 22,// >
	GE			= 23,// >=
	ASSIGN		= 24,// <-
	L_PAREN		= 25,// (
	R_PAREN		= 26,// )
	L_BRACE		= 27,// {
	R_BRACE		= 28,// }
	COMMA		= 29,// ,
	SEMICOLON	= 30,// ;
	PERIOD		= 31,// .
	IDENT		= 32,
	NUMBER		= 33
};

class Token {
	public:
		Token();
		Token(tokenType type, int ln, int cn, std::string val);
		void displayToken(int id);
		tokenType getType();
	private:
		tokenType type;
		int lineNumber;
		int columnNumber;
		std::string value;

		std::string tokenEnumStrings[34] = {
			"LET",
			"VAR",
			"IF",
			"THEN",
			"ELSE",
			"FI",
			"WHILE",
			"DO",
			"OD",
			"VOID",
			"FUNCTION",
			"CALL",
			"RETURN",
			"MAIN",
			"ADD",
			"SUB",
			"MUL",
			"DIV",
			"EQ",
			"NEQ",
			"LT",
			"LE",
			"GT",
			"GE",
			"ASSIGN",
			"L_PAREN",
			"R_PAREN",
			"L_BRACE",
			"R_BRACE",
			"COMMA",
			"SEMICOLON",
			"PERIOD",
			"IDENT",
			"NUMBER"
		};

		std::string getTextForEnum(int enumVal);

};


#endif
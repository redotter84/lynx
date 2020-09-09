#pragma once

#include "env.h"
#include "database.h"
#include "tools.h"

class Scaner
{
private:
	string _code; // variable contains code
	string compress_code(const vector<string>& c) const; // join lines to one string
	Tokens id_token(string& token) const; // recognize which token is for string
public:
	Scaner(const vector<string>& c); // constructor
	vector<Token> get_tokens() const; // replace code to tokens
};

 #pragma once

#include "env.h"
#include "database.h"
#include "tools.h"

class Parser
{
private:
	vector<Token> _code;
	vector<Token> sugar_while(const vector<Token>& tokens, 
		vector<Token>::const_iterator& iter) const;
	vector<Token> sugar_elif(const vector<Token>& tokens, 
		vector<Token>::const_iterator& iter) const;
	vector<Token> sugar(const vector<Token>& tokens) const;
	vector<Token> parse_if(const vector<Token>& code) const;
	vector<Token> parse_for(const vector<Token>& code) const;
	vector<Token> parse_def(const vector<Token>& code) const;
	vector<Token> parse_block(const vector<Token>& code) const;
	vector<Token> parse_stat(const vector<Token>& code) const;
	vector<Token> get_expr(const vector<Token>& code) const;
public:
	Parser(const vector<Token>& c);
	vector<Token> get_struct() const;
};

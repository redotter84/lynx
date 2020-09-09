#pragma once

#include "database.h"
#include "env.h"
#include "tools.h"

class SyntaxTree
{
private:
	Nodes id_type(Token token) const;
	string view(const string n) const;
public:
	string value;
	Nodes node_type;
	vector<shared_ptr<SyntaxTree>> subs;
	SyntaxTree();
	SyntaxTree(Token token);
	operator string() const;
};

SyntaxTree create_tree(vector<Token>* tokens);
string arr_str(const SyntaxTree& tree);

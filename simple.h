#pragma once

#include "env.h"
#include "tree.h"

struct Variable {
	string name;
	string value;
	string type;
	string path;
	string sub_type;
	bool is_const;
	bool is_def;
	bool operator < (const Variable& v) const;
	bool operator == (const Variable& v) const;
};

struct Function {
	string name;
	string type;
	string path;
	SyntaxTree args;
	bool operator < (const Function& f) const;
	bool operator == (const Function& f) const;
};

extern vector<Variable> rnt_var_table;
extern vector<Function> rnt_fun_table;

class SimpleTree {
private:
	void get_vars(const SyntaxTree& tree, const string path = "0", const string curp = "") const;
	void get_funs(const SyntaxTree& tree, const string path = "0") const;
	bool good_path(const string p1, const string p2) const;
	void simple_tree(SyntaxTree& tree, const string path = "0") const;
public:
	SimpleTree(SyntaxTree& tree);
};

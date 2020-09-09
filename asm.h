#pragma once

#include "database.h"
#include "env.h"
#include "simple.h"

class Asm
{
private:
	string asm_vars;
	string asm_funs;
	string asm_fun_tcode;
	string asm_tcode;
	size_t temp_count = 0;
	size_t stat_count = 0;
	void load_vars();
	void load_args(SyntaxTree& args, SyntaxTree& vals);
	bool pre_funs_asm_tree(SyntaxTree& tree);
	string arith_asm_tree(SyntaxTree& tree);
	string comp_asm_tree(SyntaxTree& tree);
	string logic_asm_tree(SyntaxTree& tree);
	string ass_asm_tree(SyntaxTree& tree);
	string arr_asm_tree(SyntaxTree& tree);
	string oper_asm_tree(SyntaxTree& tree);
	string stat_asm_tree(SyntaxTree& tree);
	string fun_asm_tree(SyntaxTree& tree);
	string asm_tree(SyntaxTree& tree);
public:
	string asm_code;
	Asm(const SyntaxTree& tree);
};


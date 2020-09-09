#include "asm.h"
#include "asm_funs.h"
#include "tools.h"

void Asm::load_vars()
{
	for (auto& i = rnt_var_table.begin(); i != rnt_var_table.end(); i++)
	{
		if (i->type == "")
		{
			continue;
		}
		string def;
		if (i->type == "num")
		{
			i->name = "var" + to_string(distance(rnt_var_table.begin(), i));
			if (i->value == "")
			{
				def = " dd 0";
			}
			else
			{
				def = " dd " + i->value;
			}
		}
		else if (i->type == "str")
		{
			i->name = "var" + to_string(distance(rnt_var_table.begin(), i));
			if (i->value == "")
			{
				def = " db 1024 dup (0)";
			}
			else if (i->value == "!str0")
			{
				def = " db 0, 0";
			}
			else if (i->value.substr(0, 4) == "!str")
			{
				def = " db " + i->value.substr(4) + ", 0";
			}
			else
			{
				def = " db " + i->value + ", 0";
			}
		}
		else if (i->type == "arr")
		{
			i->name = "var" + to_string(distance(rnt_var_table.begin(), i));
			if (i->value == "")
			{
				def = " dd 65536 dup (0)";
			}
			else
			{
				def = " dd " + i->value;
			}
		}
		else
		{
			break;
		}
		this->asm_vars += i->name + def + "\n";
	}
	for (auto& i = rnt_fun_table.begin(); i != rnt_fun_table.end(); i++)
	{
		i->name = "fun" + to_string(distance(rnt_fun_table.begin(), i));
	}
}

void Asm::load_args(SyntaxTree& args, SyntaxTree& vals)
{
	if (args.value == "arg")
	{
		this->asm_tree(vals);

		Variable var0;
		var0.name = args.subs[0]->value;
		var0 = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var0);
		Variable var1;
		var1.name = vals.value;
		var1 = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var1);

		if (var0.type != var1.type)
		{
			throw "bad argument";
		}

		if (var0.type == "num")
		{
			this->asm_tcode += "fld " + vals.value + "\n";
			this->asm_tcode += "fstp " + args.subs[0]->value + "\n";
		}
		else if (var0.type == "str")
		{
			const string st_c = to_string(this->stat_count);
			++stat_count;

			const string l = "mov ecx, lengthof " + vals.value + "\n";
			const string oper1 = "mov edi, offset " + args.subs[0]->value + "\n";
			const string oper2 = "mov esi, offset " + vals.value + "\n";
			string op = "loop" + st_c + ":\n";
			if (var0.type == "str")
			{
				op += "movsb \n";
			}
			else if (var0.type == "arr")
			{
				op += "movsd \n";
			}
			op += "dec ecx \n" "cmp ecx, 0 \n";
			op += "je end" + st_c + "\n";
			op += "mov eax, 0 \n" "cmp [esi], eax \n";
			op += "je end" + st_c + "\n";
			op += "jmp loop" + st_c + "\n";
			op += "end" + st_c + ":\n";
			this->asm_tcode += l + oper1 + oper2 + op;
		}
	}
	else if (args.value == "," && vals.value == ",")
	{
		load_args(*args.subs[0], *vals.subs[0]);
		load_args(*args.subs[1], *vals.subs[1]);
	}
	else if (args.node_type == Nodes::Expr)
	{
		load_args(*args.subs[0], vals);
	}
	return;
}

string Asm::arith_asm_tree(SyntaxTree& tree)
{
	string asm_ccode;

	const bool binary = db_un_oper.find(tree.value) == db_un_oper.cend();
	if (binary)
	{
		this->asm_tree(*tree.subs[0]);
		this->asm_tree(*tree.subs[1]);

		Variable var0;
		var0.name = tree.subs[0]->value;
		var0 = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var0);
		Variable var1;
		var1.name = tree.subs[1]->value;
		var1 = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var1);
		if (var0.type != "num" || var1.type != "num")
		{
			throw "NaN";
		}

		const string oper1 = "fld " + tree.subs[0]->value + "\n";
		const string oper2 = "fld " + tree.subs[1]->value + "\n";

		string op;

		if (tree.value == "+")
		{
			op = "fadd \n";
		}
		else if (tree.value == "-")
		{
			op = "fsub \n";
		}
		else if (tree.value == "*")
		{
			op = "fmul \n";
		}
		else if (tree.value == "/")
		{
			op = "fdiv \n";
		}
		else if (tree.value == "%")
		{
			op = "fxch \n" "fprem \n";
		}
		else if (tree.value == "^")
		{
			op = "fyl2x \n" "fld1 \n" "fld st(1) \n" "fprem \n" "f2xm1 \n" "fadd \n" "fscale \n";
		}
		else
		{
			return "";
		}

		const string tvar = "t" + to_string(this->temp_count);
		++this->temp_count;
		const string temp = "fstp " + tvar + "\n";

		this->asm_vars += tvar + " dd ?" + "\n";
		Variable v; v.name = tvar; v.type = "num";
		rnt_var_table.push_back(v);

		asm_ccode = oper1 + oper2 + op + temp + "fstp st(0) \n";

		tree.value = tvar;
		tree.node_type = Nodes::Var;
		tree.subs = {};
	}
	else
	{
		this->asm_tree(*tree.subs[0]);

		Variable var0;
		var0.name = tree.subs[0]->value;
		var0 = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var0);
		if (var0.type != "num")
		{
			throw "NaN";
		}

		string oper1 = "fld " + tree.subs[0]->value + "\n";

		string op;
		if (tree.value == "--")
		{
			op = "fchs \n";
		}
		else
		{
			return "";
		}

		const string tvar = "t" + to_string(this->temp_count);
		const string temp = "fstp " + tvar + "\n";
		++this->temp_count;

		this->asm_vars += tvar + " dd ?" + "\n";
		Variable v; v.name = tvar; v.type = "num";
		rnt_var_table.push_back(v);

		asm_ccode += oper1 + op + temp;

		tree.value = tvar;
		tree.node_type = Nodes::Var;
		tree.subs = {};
	}

	return asm_ccode;
}

string Asm::comp_asm_tree(SyntaxTree& tree)
{
	string asm_ccode;

	this->asm_tree(*tree.subs[0]);
	this->asm_tree(*tree.subs[1]);

	Variable var0;
	var0.name = tree.subs[0]->value;
	var0 = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var0);
	Variable var1;
	var1.name = tree.subs[1]->value;
	var1 = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var1);
	if (var0.type != var1.type)
	{
		throw "NaN";
	}

	string oper1;
	string oper2;
	string op;

	if (var0.type == "num")
	{
		oper1 = "fld " + tree.subs[1]->value + "\n";
		oper2 = "fld " + tree.subs[0]->value + "\n";
		op = "fcomi st(0), st(1) \n";
		op += "fstp st(0) \n" "fstp st(0) \n";
	}
	else if (var0.type == "str" || var0.type == "arr")
	{
		const string st_c = to_string(this->stat_count);
		++this->stat_count;

		oper1 = "mov esi, offset " + tree.subs[0]->value + "\n";
		oper2 = "mov edi, offset " + tree.subs[1]->value + "\n";
		op += "pusha \n" "mov ecx, lengthof " + tree.subs[0]->value + "\n";
		op += "loop" + st_c + ":\n";
		op += "cmpsb \n";
		op += "jne neq_str" + st_c + "\n";
		op += "mov eax, 0 \n" "cmp [esi], eax \n";
		op += "jne loop" + st_c + "\n";
		op += "cmp ecx, 0 \n";
		op += "jne loop" + st_c + "\n";
		op += "pusha \n" "pushf \n";
		op += "mov eax, lengthof " + tree.subs[0]->value + "\n";
		op += "mov ebx, lengthof " + tree.subs[1]->value + "\n";
		op += "cmp eax, ebx \n";
		op += "jne neq" + st_c + "\n";
		op += "popf \n";
		op += "jmp neq_str" + st_c + "\n";
		op += "neq" + st_c + ":\n";
		op += "popf \n";
		op += "cmp eax, ebx \n";
		op += "neq_str" + st_c + ": popa \n";
	}

	const string st_c = to_string(this->stat_count);
	++this->stat_count;

	string pcond;

	if (tree.value == "<")
	{
		pcond = "jl true" + st_c + "\n";
		pcond += "jge false" + st_c + "\n";
	}
	else if (tree.value == ">")
	{
		pcond = "jg true" + st_c + "\n";
		pcond += "jle false" + st_c + "\n";
	}
	else if (tree.value == "<=")
	{
		pcond = "jle true" + st_c + "\n";
		pcond += "jg false" + st_c + "\n";
	}
	else if (tree.value == ">=")
	{
		pcond = "jge true" + st_c + "\n";
		pcond += "jl false" + st_c + "\n";
	}
	else if (tree.value == "==")
	{
		pcond = "je true" + st_c + "\n";
		pcond += "jne false" + st_c + "\n";
	}
	else if (tree.value == "/=")
	{
		pcond = "jne true" + st_c + "\n";
		pcond += "je false" + st_c + "\n";
	}
	else
	{
		throw "bad operation";
	}

	const string ptrue = "true" + st_c + ": fld1 \n" + "jmp end" + st_c + "\n";
	const string pfalse = "false" + st_c + ": fldz \n";
	const string pend = "end" + st_c + ":\n";

	const string tvar = "t" + to_string(this->temp_count);
	++this->temp_count;
	
	const string temp = "fstp " + tvar + "\n";
	this->asm_vars += tvar + " dd ?" + "\n";
	Variable v; v.name = tvar; v.type = "num";
	rnt_var_table.push_back(v);
		
	asm_ccode = oper1 + oper2 + op + pcond + ptrue + pfalse + pend + temp;
	
	tree.value = tvar;
	tree.node_type = Nodes::Var;
	tree.subs = {};

	return asm_ccode;
}

string Asm::logic_asm_tree(SyntaxTree& tree)
{
	string asm_ccode;

	const bool binary = db_un_oper.find(tree.value) == db_un_oper.cend();
	if (binary)
	{
		this->asm_tree(*tree.subs[0]);
		this->asm_tree(*tree.subs[1]);

		Variable var0;
		var0.name = tree.subs[0]->value;
		var0 = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var0);
		Variable var1;
		var1.name = tree.subs[1]->value;
		var1 = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var1);
		if (var0.type != "num" || var1.type != "num")
		{
			throw "NaN";
		}

		const string oper1 = "fld " + tree.subs[0]->value + "\n";
		const string oper2 = "fld " + tree.subs[1]->value + "\n";

		const string tvar1 = "t" + to_string(this->temp_count);
		++this->temp_count;
		const string tvar2 = "t" + to_string(this->temp_count);
		++this->temp_count;

		string reg1 = "fistp " + tvar1 + "\n";
		reg1 += "mov ax, " + tvar1 + "\n";

		string reg2 = "fistp " + tvar1 + "\n";
		reg2 += "mov bx, " + tvar1 + "\n";

		const string op = tree.value + " ax, bx \n";

		string load = "mov " + tvar2 + ", ax \n";
		load += "fild " + tvar2 + "\n";

		const string tvar = "t" + to_string(this->temp_count);
		++this->temp_count;
		const string temp = "fstp " + tvar + "\n";

		this->asm_vars += tvar1 + " dw ?" + "\n";
		this->asm_vars += tvar2 + " dw ?" + "\n";
		this->asm_vars += tvar + " dd ?" + "\n";
		
		Variable v; v.name = tvar1; v.type = "num";
		rnt_var_table.push_back(v);
		v.name = tvar2; v.type = "num";
		rnt_var_table.push_back(v);
		v.name = tvar; v.type = "num";
		rnt_var_table.push_back(v);

		asm_ccode = oper1 + oper2 + reg1 + reg2 + op + load + temp + 
			"fstp st(0) \n";

		tree.value = tvar;
		tree.node_type = Nodes::Var;
		tree.subs = {};
	}
	else if (tree.value == "not")
	{
		this->asm_tree(*tree.subs[0]);

		Variable var0;
		var0.name = tree.subs[0]->value;
		var0 = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var0);
		if (var0.type != "num")
		{
			throw "NaN";
		}

		string oper1 = "fld " + tree.subs[0]->value + "\n";
		const string load01 = "fld1 \n" "fldz \n";

		string op = "fcomi st(0), st(1) \n" "fcmove st(0), st(2) \n" "fcmovne st(0), st(1) \n";
		
		string tvar = "t" + to_string(this->temp_count);
		const string temp = "fstp " + tvar + "\n";
		++this->temp_count;

		this->asm_vars += tvar + " dd ?" + "\n";
		Variable v; v.name = tvar; v.type = "num";
		rnt_var_table.push_back(v);

		asm_ccode += load01 + oper1 + op + temp;

		tree.value = tvar;
		tree.node_type = Nodes::Var;
		tree.subs = {};
	}
	else
	{
		return "";
	}

	return asm_ccode;
}

string Asm::ass_asm_tree(SyntaxTree& tree)
{
	string asm_ccode;

	this->asm_tree(*tree.subs[0]);
	this->asm_tree(*tree.subs[1]);

	Variable var0;
	var0.name = tree.subs[0]->value;
	var0 = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var0);
	Variable var1;
	var1.name = tree.subs[1]->value;
	var1 = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var1);

	if (var0.type != var1.type)
	{
		throw "bad type casting";
	}

	if (var0.is_const)
	{
		if (var0.is_def)
		{
			throw "const reassignment";
		}
		else
		{
			find(rnt_var_table.begin(), rnt_var_table.end(), var0)->is_def = true;
		}
	}

	if (var0.type == "num")
	{
		const string temp = "fld " + tree.subs[1]->value + "\n";
		const string op = "fstp " + tree.subs[0]->value + "\n";
		asm_ccode = temp + op;
	}
	else if (var0.type == "str")
	{
		const string st_c = to_string(this->stat_count);
		++this->stat_count;

		const string len = "mov ecx, lengthof " + tree.subs[1]->value + "\n";
		const string oper1 = "mov edi, offset " + tree.subs[0]->value + "\n";
		const string oper2 = "mov esi, offset " + tree.subs[1]->value + "\n";

		string op = "loop" + st_c + ":\n";
		if (var0.type == "str")
		{
			op += "movsb \n";
		}
		else if (var0.type == "arr")
		{
			op += "movsd \n";
		}
		op += "dec ecx \n" "cmp ecx, 0 \n";
		op += "je end" + st_c + "\n";
		op += "mov eax, 0 \n" "cmp [esi], eax \n";
		op += "je end" + st_c + "\n";
		op += "jmp loop" + st_c + "\n";
		op += "end" + st_c + ":\n";

		asm_ccode = len + oper1 + oper2 + op;
	}
	else if (var0.type == "arr")
	{
		const string len = "mov ecx, lengthof " + tree.subs[1]->value + "\n";
		const string oper1 = "mov edi, offset " + tree.subs[0]->value + "\n";
		const string oper2 = "mov esi, offset " + tree.subs[1]->value + "\n";
		const string op = "rep movsd \n";
		asm_ccode = len + oper1 + oper2 + op;
		find(rnt_var_table.begin(), rnt_var_table.end(), var0)->sub_type = var1.sub_type;
	}

	tree.value = tree.subs[0]->value;
	tree.node_type = Nodes::Var;
	tree.subs = {};

	return asm_ccode;
}

string Asm::arr_asm_tree(SyntaxTree& tree)
{
	string asm_ccode;

	if (tree.value == "@")
	{
		this->asm_tree(*tree.subs[0]);
		this->asm_tree(*tree.subs[1]);

		Variable var0;
		var0.name = tree.subs[0]->value;
		var0 = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var0);
		Variable var1;
		var1.name = tree.subs[1]->value;
		var1 = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var1);
		if (var0.type != "str" && var0.type != "arr" || var1.type != "num")
		{
			throw "bad operands";
		}

		if (var0.type == "str" || var0.type == "arr")
		{				
			const string tvar1 = "t" + to_string(this->temp_count);
			++this->temp_count;
			
			string load = "fld " + tree.subs[1]->value + "\n";
			load += "fstp " + tvar1 + "\n";
			load += "mov eax, " + tvar1 + "\n";

			const string tvar = "t" + to_string(this->temp_count);
			++this->temp_count;

			string temp;
			string op;
			if (var0.type == "str")
			{
				op = "mov ah, " + tree.subs[0]->value + "[eax] \n";
				temp = "mov " + tvar + ", ah \n";
			}
			else if (var0.type == "arr")
			{
				op = "mov eax, " + tree.subs[0]->value + "[eax] \n";
				temp = "mov " + tvar + ", eax \n";
			}

			this->asm_vars += tvar1 + " dd ?" + "\n";
			if (var0.type == "str")
			{
				this->asm_vars += tvar + " db ?, 0" + "\n";
			}
			else if (var0.type == "arr")
			{
				this->asm_vars += tvar + " dd ?" + "\n";
			}

			Variable v; v.name = tvar; 
			if (var0.type == "str")
			{
				v.type = "str";
			}
			else if (var0.type == "arr")
			{
				v.type = var0.sub_type;
			}
			rnt_var_table.push_back(v);

			asm_ccode = load + op + temp;

			tree.value = tvar;
			tree.node_type = Nodes::Var;
			tree.subs = {};
		}
	}
	else if (tree.value == "++")
	{
		string st_c = to_string(this->stat_count);
		++this->stat_count;

		this->asm_tree(*tree.subs[0]);
		this->asm_tree(*tree.subs[1]);

		Variable var0;
		var0.name = tree.subs[0]->value;
		var0 = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var0);
		Variable var1;
		var1.name = tree.subs[1]->value;
		var1 = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var1);
		if (var0.type != var1.type)
		{
			throw "bad operands";
		}
		if (var0.type != "str" && var0.type != "arr")
		{
			throw "bad type casting";
		}

		if (var0.type == "str" || var0.type == "arr")
		{
			const string tvar = "t" + to_string(this->temp_count);
			++this->temp_count;
			
			string op = "loop" + st_c + ":\n";
			if (var0.type == "str")
			{
				op += "movsb \n";
			}
			else if (var0.type == "arr")
			{
				op += "movsd \n";
			}
			op += "mov edx, edi \n" "dec ecx \n" "cmp ecx, 0 \n";
			op += "je end" + st_c + "\n";
			op += "mov eax, 0 \n" "cmp [esi], eax \n";
			op += "je end" + st_c + "\n";
			op += "jmp loop" + st_c + "\n";
			op += "end" + st_c + ":\n";

			string part1 = "mov ecx, lengthof " + tree.subs[0]->value + "\n";
			part1 += "mov esi, offset " + tree.subs[0]->value + "\n";
			part1 += "mov edi, offset " + tvar + "\n";
			part1 += op;

			st_c = to_string(this->stat_count);
			++this->stat_count;

			op = "loop" + st_c + ":\n";
			if (var0.type == "str")
			{
				op += "movsb \n";
			}
			else if (var0.type == "arr")
			{
				op += "movsd \n";
			}
			op += "dec ecx \n" "cmp ecx, 0 \n";
			op += "je end" + st_c + "\n";
			op += "mov eax, 0 \n" "cmp [esi], eax \n";
			op += "je end" + st_c + "\n";
			op += "jmp loop" + st_c + "\n";
			op += "end" + st_c + ":\n";

			string part2 = "mov ecx, lengthof " + tree.subs[1]->value + "\n";
			part2 += "mov esi, offset " + tree.subs[1]->value + "\n";
			part2 += "mov edi, edx \n";
			part2 += op;

			if (var0.type == "str")
			{
				this->asm_vars += tvar + " db (sizeof " + tree.subs[0]->value + " + sizeof " +
					tree.subs[1]->value + ") dup (0)" + "\n";
			}
			else if (var0.type == "arr")
			{
				this->asm_vars += tvar + " dd (sizeof " + tree.subs[0]->value + " + sizeof " +
					tree.subs[1]->value + ") dup (?)" + "\n";
			}
			
			Variable v; v.name = tvar; v.type = "str";
			rnt_var_table.push_back(v);

			asm_ccode += part1 + part2;

			tree.value = tvar;
			tree.node_type = Nodes::Var;
			tree.subs = {};
		}
	}
	else
	{
		throw "bad operator";
	}

	return asm_ccode;
}

string Asm::oper_asm_tree(SyntaxTree& tree)
{
	string asm_ccode;

	const bool let = db_let_oper.find(tree.value) != db_let_oper.cend();
	const bool arith = db_arith_oper.find(tree.value) != db_arith_oper.cend();
	const bool comp = db_comp_oper.find(tree.value) != db_comp_oper.cend();
	const bool logic = db_logic_oper.find(tree.value) != db_logic_oper.cend();
	const bool arr = db_arr_oper.find(tree.value) != db_arr_oper.cend();

	if (tree.value == "=")
	{
		asm_ccode = this->ass_asm_tree(tree);
	}
	else if (tree.value == ",")
	{
		this->asm_tree(*tree.subs[0]);
		this->asm_tree(*tree.subs[1]);

		const string tvar = "t" + to_string(this->temp_count);
		++this->temp_count;

		Variable var0;
		var0.name = tree.subs[0]->value;
		var0 = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var0);

		this->asm_vars += tvar + " dd " + arr_str(tree) + "\n";
		Variable v; v.name = tvar; v.type = "arr"; v.sub_type = var0.type;
		rnt_var_table.push_back(v);

		tree.value = tvar;
		tree.node_type = Nodes::Var;
		tree.subs = {};

		this->asm_tree(tree);
	}
	else if (let)
	{
		this->asm_tree(*tree.subs[0]);
		this->asm_tree(*tree.subs[1]);

		tree.value = tree.subs[0]->value;
		tree.node_type = Nodes::Var;
		tree.subs = {};
	}
	else if (tree.value == "return")
	{
		this->asm_tree(*tree.subs[0]);

		Variable var0;
		var0.name = tree.subs[0]->value;
		var0 = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var0);

		if (var0.type == "num")
		{
			asm_ccode += "fld " + tree.subs[0]->value + "\n";
		}
		else if (var0.type == "str")
		{
			asm_ccode += "mov ecx, lengthof " + tree.subs[0]->value + "\n";
			asm_ccode += "mov esi, offset " + tree.subs[0]->value + "\n";
		}

		asm_ccode += "ret \n";

		tree.value = tree.subs[0]->value;
		tree.node_type = Nodes::Var;
		tree.subs = {};
	}
	else if (arith)
	{
		asm_ccode = arith_asm_tree(tree);
	}
	else if (comp)
	{
		asm_ccode = comp_asm_tree(tree);
	}
	else if (logic)
	{
		asm_ccode = logic_asm_tree(tree);
	}
	else if (arr)
	{
		asm_ccode = arr_asm_tree(tree);
	}
	return asm_ccode;
}

string Asm::stat_asm_tree(SyntaxTree& tree)
{
	if (tree.value == "if")
	{
		const string st_c = to_string(this->stat_count);
		++this->stat_count;

		this->asm_tree(*tree.subs[0]);
		this->asm_tcode += "fld " + tree.subs[0]->value + "\n";

		string if_p = "fldz \n" "fcomi st(0), st(1) \n" "fstp st(0) \n" "fstp st(0) \n";
		if_p += "jne then" + st_c + "\n";
		if_p += "je else" + st_c + "\n";

		this->asm_tcode += if_p + "then" + st_c + ":\n";
		this->asm_tree(*tree.subs[1]);

		Variable var;
		var.name = tree.subs[1]->value;
		var = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var);
		if (var.type == "num")
		{
			this->asm_tcode += "fld " + tree.subs[1]->value + "\n";
		}
		else if (var.type == "str")
		{
			this->asm_tcode += "mov ecx, lengthof " + tree.subs[1]->value + "\n";
			this->asm_tcode += "mov esi, offset " + tree.subs[1]->value + "\n";
		}

		const string then_p = "jmp end_if" + st_c + "\n";

		this->asm_tcode += then_p + "else" + st_c + ":\n";
		this->asm_tree(*tree.subs[2]);

		if (var.type == "num")
		{
			this->asm_tcode += "fld " + tree.subs[2]->value + "\n";
		}
		else if (var.type == "str")
		{
			this->asm_tcode += "mov ecx, lengthof " + tree.subs[1]->value + "\n";
			this->asm_tcode += "mov esi, offset " + tree.subs[1]->value + "\n";
		}

		const string end_p = "end_if" + st_c + ":\n";

		const string tvar = "t" + to_string(this->temp_count);
		++this->temp_count;
		
		string temp;
		if (var.type == "num")
		{
			temp = "fstp " + tvar + "\n";
			this->asm_vars += tvar + " dd ?" + "\n";
			Variable v; v.name = tvar; v.type = "num";
			rnt_var_table.push_back(v);
		}
		else if (var.type == "str")
		{
			temp += "mov edi, offset " + tvar + "\n";
			temp += "rep movsb \n";
			this->asm_vars += tvar + " db 1024 dup (0)" + "\n";
			Variable v; v.name = tvar; v.type = "str";
			rnt_var_table.push_back(v);
		}

		this->asm_tcode += end_p + temp;

		tree.value = tvar;
		tree.node_type = Nodes::Var;
		tree.subs = {};
	}
	else if (tree.value == "for")
	{
		const string st_c = to_string(this->stat_count);
		++this->stat_count;

		this->asm_tcode += "fldz \n";
		this->asm_tree(*tree.subs[0]);
		
		this->asm_tcode += "loop" + st_c + ":\n";
		this->asm_tree(*tree.subs[1]);
		this->asm_tcode += "fld " + tree.subs[1]->value + "\n";
		
		string cond_p = "fldz \n" "fcomi st(0), st(1) \n" "fstp st(0) \n" "fstp st(0) \n";
		cond_p += "je end_loop" + st_c + "\n";
		cond_p += "fstp st(0) \n";

		this->asm_tcode += cond_p;
		this->asm_tree(*tree.subs[3]);

		Variable var;
		var.name = tree.subs[3]->value;
		var = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var);
		if (var.type == "num")
		{
			this->asm_tcode += "fld " + tree.subs[3]->value + "\n";
		}
		else if (var.type == "str")
		{
			this->asm_tcode += "mov ecx, lengthof " + tree.subs[1]->value + "\n";
			this->asm_tcode += "mov esi, offset " + tree.subs[1]->value + "\n";
		}

		this->asm_tree(*tree.subs[2]);
		
		string end_p = "jmp loop" + st_c + "\n";
		end_p += "end_loop" + st_c + ":\n";
		
		const string tvar = "t" + to_string(this->temp_count);
		++this->temp_count;
		
		string temp;
		if (var.type == "num")
		{
			temp = "fstp " + tvar + "\n";
			this->asm_vars += tvar + " dd ?" + "\n";
			Variable v; v.name = tvar; v.type = "num";
			rnt_var_table.push_back(v);
		}
		else if (var.type == "str")
		{
			temp += "mov edi, offset " + tvar + "\n";
			temp += "rep movsb \n";
			this->asm_vars += tvar + " db 1024 dup (0)" + "\n";
			Variable v; v.name = tvar; v.type = "str";
			rnt_var_table.push_back(v);
		}

		this->asm_tcode += end_p + temp;

		tree.value = tvar;
		tree.node_type = Nodes::Var;
		tree.subs = {};
	}
	else if (tree.value == "def")
	{
		const string st_c = to_string(this->stat_count);
		++this->stat_count;

		Function fun;
		this->asm_tree(*tree.subs[0]);
		fun.name = tree.subs[0]->value;
		if (!tree.subs[1]->subs.empty())
		{
			find(rnt_fun_table.begin(), rnt_fun_table.end(), fun)->args = *tree.subs[1]->subs[0];
		}
		else
		{
			find(rnt_fun_table.begin(), rnt_fun_table.end(), fun)->args = 
				(SyntaxTree(Token(Tokens::Lit, "nil")));
		}

		this->asm_fun_tcode += tree.subs[0]->value + " proc \n";
		const string temp_code = this->asm_tcode;
		this->asm_tree(*tree.subs[3]);
		this->asm_fun_tcode += this->asm_tcode;
		this->asm_tcode = temp_code;
		this->asm_fun_tcode += tree.subs[0]->value + " endp \n";
	}
	else
	{
		return "";
	}
	return "";
}

string Asm::fun_asm_tree(SyntaxTree& tree)
{
	Function fun;
	fun.name = tree.value;

	if (this->pre_funs_asm_tree(tree))
	{
		return "";
	}

	fun = *find(rnt_fun_table.begin(), rnt_fun_table.end(), fun);
	SyntaxTree args = fun.args;

	this->load_args(args, *tree.subs[0]);
	asm_tcode += "call " + tree.value + "\n";

	const string tvar = "t" + to_string(this->temp_count);
	++this->temp_count;
	string temp;
	if (fun.type == "num")
	{
		temp = "fstp " + tvar + "\n";
		this->asm_vars += tvar + " dd ?" + "\n";
		Variable v; v.name = tvar; v.type = "num";
		rnt_var_table.push_back(v);
	}
	else if (fun.type == "str")
	{
		this->asm_vars += tvar + " db 1024 dup (0)" + "\n";
		Variable v; v.name = tvar; v.type = "str";
		rnt_var_table.push_back(v);
	}

	asm_tcode += temp;

	tree.value = tvar;
	tree.node_type = Nodes::Var;
	tree.subs = {};

	return "";
}

string Asm::asm_tree(SyntaxTree& tree)
{
	if (tree.node_type == Nodes::Var || tree.node_type == Nodes::FunName)
	{
		return "";
	}
	else if (tree.node_type == Nodes::Lit)
	{
		const string tvar = "t" + to_string(this->temp_count);
		++this->temp_count;

		if (is_num(tree.value))
		{
			string name = "lit" + tree.value + " ";
			replace_all(name, ".", "p");
			replace_all(name, "-", "m");
			if (this->asm_vars.find(name) == string::npos)
			{
				this->asm_vars += name + "dd " + tree.value + "\n";
				Variable v; v.name = name; v.type = "num";
				rnt_var_table.push_back(v);
			}
			tree.value = name;
			tree.node_type = Nodes::Var;
			tree.subs = {};
		}
		else if (tree.value[0] == '"')
		{
			this->asm_vars += tvar + " db " + tree.value + ", 0 \n";
			Variable v; v.name = tvar; v.type = "str";
			rnt_var_table.push_back(v);

			tree.value = tvar;
			tree.node_type = Nodes::Var;
			tree.subs = {};
		}
		else if (tree.value == "!str0")
		{
			this->asm_vars += tvar + " db 0, 0 \n";
			Variable v; v.name = tvar; v.type = "str";
			rnt_var_table.push_back(v);

			tree.value = tvar;
			tree.node_type = Nodes::Var;
			tree.subs = {};
		}
		else if (tree.value.substr(0, 4) == "!str")
		{
			this->asm_vars += tvar + " db " + tree.value.substr(4) + ", 0 \n";
			Variable v; v.name = tvar; v.type = "str";
			rnt_var_table.push_back(v);

			tree.value = tvar;
			tree.node_type = Nodes::Var;
			tree.subs = {};
		}
		else if (tree.value == "nil")
		{
			this->asm_vars += tvar + " dd 0" + "\n";
			Variable v; v.name = tvar; v.type = "num";
			rnt_var_table.push_back(v);

			tree.value = tvar;
			tree.node_type = Nodes::Var;
			tree.subs = {};
		}

		return "";
	}
	else if (tree.node_type == Nodes::Oper)
	{
		this->asm_tcode += oper_asm_tree(tree);
		return "";
	}
	else if (tree.node_type == Nodes::Expr)
	{
		const bool is_ret = tree.subs[0]->value == "return";

		this->asm_tree(*tree.subs[0]);

		if (tree.value != "def-name" && !is_ret)
		{
			Variable var;
			var.name = tree.subs[0]->value;
			var = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var);
			if (var.type == "num")
			{
				this->asm_tcode += "fstp st(0) \n";
			}
			else if (var.type == "str")
			{
				this->asm_tcode += "mov ecx, lengthof " + tree.subs[0]->value + "\n";
				this->asm_tcode += "mov esi, offset " + tree.subs[0]->value + "\n";
			}
		}

		tree.value = tree.subs[0]->value;
		tree.node_type = Nodes::Var;
		tree.subs = {};
		return "";
	}
	else if (tree.node_type == Nodes::Block)
	{
		for (auto& t : tree.subs)
		{
			this->asm_tree(*t);
		}
		tree.value = tree.subs[tree.subs.size() - 1]->value;
		tree.node_type = Nodes::Var;
		tree.subs = {};
		return "";
	}
	else if (tree.node_type == Nodes::Stat)
	{
		this->stat_asm_tree(tree);
		return "";
	}
	else if (tree.node_type == Nodes::Fun)
	{
		this->fun_asm_tree(tree);
		return "";
	}
	else if (tree.node_type == Nodes::Program)
	{
		for (auto& t : tree.subs)
		{
			asm_tree(*t);
		}

		asm_code += ".686 \n";
		asm_code += ".model flat, stdcall \n";
		asm_code += "option casemap:none \n";
		asm_code += "include C:\\masm32\\include\\windows.inc \n";
		asm_code += "include C:\\masm32\\include\\kernel32.inc \n";
		asm_code += "includelib C:\\masm32\\lib\\kernel32.lib \n";
		asm_code += ".stack 100h \n";
		asm_code += ".data \n";
		asm_code += this->asm_vars;
		asm_code += "h_in dd ? \n";
		asm_code += "h_out dd ? \n";
		asm_code += ".code \n";
		asm_code += this->asm_fun_tcode;
		asm_code += "_start: \n";
		asm_code += "invoke GetStdHandle, STD_INPUT_HANDLE \n";
		asm_code += "mov h_in, eax \n";
		asm_code += "invoke GetStdHandle, STD_OUTPUT_HANDLE \n";
		asm_code += "mov h_out, eax \n";
		asm_code += this->asm_tcode;
		asm_code += "invoke ExitProcess, 0 \n";
		asm_code += "end _start \n";
		return asm_code;
	}
	else
	{
		for (auto& t : tree.subs)
		{
			this->asm_tree(*t);
		}
		return "";
	}
}

Asm::Asm(const SyntaxTree& tree)
{
	this->load_vars();
	SyntaxTree _tree = tree;
	this->asm_tree(_tree);
}

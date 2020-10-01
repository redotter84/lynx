#pragma once

#include "env.h"

#include "asm.h"

bool Asm::pre_funs_asm_tree(SyntaxTree& tree) {
	if (tree.value == "fun0") // print
	{
		const string st_c = to_string(this->stat_count);
		++this->stat_count;

		this->asm_tree(*tree.subs[0]);

		const string a1 = tree.subs[0]->value;

		Variable var0;
		var0.name = a1;
		var0 = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var0);
		if (var0.type != "str") {
			throw "bad arguments";
		}

		this->asm_tcode += "mov esi, offset " + a1 + "\n";
		this->asm_tcode += "_print_l" + st_c + ":\n";
		this->asm_tcode += "pusha \n";
		this->asm_tcode += "invoke WriteConsole, h_out, esi, 1, ebx, NULL \n" "popa \n"
			"inc esi \n" "mov eax, 0 \n" "cmp [esi], al \n";
		this->asm_tcode += "jne _print_l" + st_c + "\n";

		this->asm_tcode += "mov ecx, sizeof " + a1 + "\n";
		this->asm_tcode += "mov esi, offset " + a1 + "\n";

		tree.value = tree.subs[0]->value;
		tree.node_type = Nodes::Var;
		tree.subs = {};

		return true;
	} else if (tree.value == "fun1") // input
	{
		const string tvar = "t" + to_string(this->temp_count);
		++this->temp_count;

		this->asm_vars += tvar + " db 1024 dup (0)" + "\n";
		Variable v; v.name = tvar; v.type = "str";
		rnt_var_table.push_back(v);

		this->asm_tcode += "pusha \n";
		this->asm_tcode += "invoke ReadConsole, h_in, addr " + tvar + ", sizeof " + tvar +
			", ebx, NULL \n";
		this->asm_tcode += "mov ebx, [ebx] \n";
		this->asm_tcode += "sub ebx, 2 \n";
		this->asm_tcode += "mov al, 0 \n";
		this->asm_tcode += "mov " + tvar + "[ebx], al \n";
		this->asm_tcode += "mov " + tvar + "[ebx+1], al \n";
		this->asm_tcode += "popa \n";

		tree.value = tvar;
		tree.node_type = Nodes::Var;
		tree.subs = {};

		return true;
	} else if (tree.value == "fun2") // sin
	{
		this->asm_tree(*tree.subs[0]);

		Variable var0;
		var0.name = tree.subs[0]->value;
		var0 = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var0);
		if (var0.type != "num") {
			throw "bad arguments";
		}

		const string oper1 = "fld " + tree.subs[0]->value + "\n";
		const string op = "fsin \n";

		const string tvar = "t" + to_string(this->temp_count);
		++this->temp_count;
		const string temp = "fstp " + tvar + "\n";

		this->asm_vars += tvar + " dd ?" + "\n";
		Variable v; v.name = tvar; v.type = "num";
		rnt_var_table.push_back(v);

		this->asm_tcode += oper1 + op + temp;

		tree.value = tvar;
		tree.node_type = Nodes::Var;
		tree.subs = {};

		return true;
	} else if (tree.value == "fun3") // cos
	{
		this->asm_tree(*tree.subs[0]);

		Variable var0;
		var0.name = tree.subs[0]->value;
		var0 = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var0);
		if (var0.type != "num") {
			throw "bad arguments";
		}

		const string oper1 = "fld " + tree.subs[0]->value + "\n";
		const string op = "fsin \n";

		const string tvar = "t" + to_string(this->temp_count);
		++this->temp_count;
		const string temp = "fstp " + tvar + "\n";

		this->asm_vars += tvar + " dd ?" + "\n";
		Variable v; v.name = tvar; v.type = "num";
		rnt_var_table.push_back(v);

		this->asm_tcode += oper1 + op + temp;

		tree.value = tvar;
		tree.node_type = Nodes::Var;
		tree.subs = {};

		return true;
	} else if (tree.value == "fun4") // tan
	{
		this->asm_tree(*tree.subs[0]);

		Variable var0;
		var0.name = tree.subs[0]->value;
		var0 = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var0);
		if (var0.type != "num") {
			throw "bad arguments";
		}

		const string oper1 = "fld " + tree.subs[0]->value + "\n";
		const string op = "fptan \n" "fstp st(0) \n";

		const string tvar = "t" + to_string(this->temp_count);
		++this->temp_count;
		const string temp = "fstp " + tvar + "\n";

		this->asm_vars += tvar + " dd ?" + "\n";
		Variable v; v.name = tvar; v.type = "num";
		rnt_var_table.push_back(v);

		this->asm_tcode += oper1 + op + temp;

		tree.value = tvar;
		tree.node_type = Nodes::Var;
		tree.subs = {};

		return true;
	} else if (tree.value == "fun5") // log
	{
		this->asm_tree(*tree.subs[0]);

		Variable var0;
		var0.name = tree.subs[0]->value;
		var0 = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var0);
		if (var0.type != "num") {
			throw "bad arguments";
		}

		string op = "fldl2e \n" "fld1 \n";
		op += "fld " + tree.subs[0]->value + "\n";
		op += "fyl2x \n" "fdiv \n";

		const string tvar = "t" + to_string(this->temp_count);
		++this->temp_count;
		const string temp = "fstp " + tvar + "\n";

		this->asm_vars += tvar + " dd ?" + "\n";
		Variable v; v.name = tvar; v.type = "num";
		rnt_var_table.push_back(v);

		this->asm_tcode += op + temp + "fstp st(0) \n";

		tree.value = tvar;
		tree.node_type = Nodes::Var;
		tree.subs = {};

		return true;
	} else if (tree.value == "fun6") // round
	{
		this->asm_tree(*tree.subs[0]);

		Variable var0;
		var0.name = tree.subs[0]->value;
		var0 = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var0);
		if (var0.type != "num") {
			throw "bad arguments";
		}

		const string oper1 = "fld " + tree.subs[0]->value + "\n";
		const string op = "frndint \n";

		const string tvar = "t" + to_string(this->temp_count);
		++this->temp_count;
		const string temp = "fistp " + tvar + "\n";

		this->asm_vars += tvar + " dd ?" + "\n";
		Variable v; v.name = tvar; v.type = "num";
		rnt_var_table.push_back(v);

		this->asm_tcode += oper1 + op + temp;

		tree.value = tvar;
		tree.node_type = Nodes::Var;
		tree.subs = {};

		return true;
	} else if (tree.value == "fun7") // str
	{
		const string tvar = "t" + to_string(this->temp_count);
		++this->temp_count;

		const string st_c = to_string(this->stat_count);
		++this->stat_count;

		this->asm_tree(*tree.subs[0]);
		const string a1 = tree.subs[0]->value;

		Variable var0;
		var0.name = a1;
		var0 = *find(rnt_var_table.cbegin(), rnt_var_table.cend(), var0);
		if (var0.type != "num") {
			throw "bad arguments";
		}

		static bool was = false;
		string proc = R"(_str proc
cmp eax, 0
jne _str_beg
mov esi, offset _str_zero
movsb
ret
_str_beg:
push edi
cmp eax, 0
jge _str_all
neg eax
mov esi, offset _str_min
movsb
pop esi
push edi
xor esi, esi
_str_all:
mov ecx, 0
mov edi, offset _str_temp
_str_l1:
inc ecx
xor edx, edx
div _str_ten
add dl, 48
mov [edi], dl
inc edi
cmp eax, 0
jne _str_l1
dec edi
dec ecx
mov esi, edi
pop edi
_str_l2:
movsb
dec esi
mov eax, 0
mov [esi], eax
dec esi
dec ecx
cmp ecx, -1
jne _str_l2
ret
_str endp
)";
		string vars = R"(_str_int dd ?
_str_fract dd ?
_str_pow dd 1000000000
_str_ten dd 10
_str_min db '-'
_str_dot db '.'
_str_zero db '0'
_str_temp db 1024 dup(0)
_str_res db 1024 dup(0)
)";
		if (was) {
			proc = "";
			vars = "";
		} else {
			was = true;
		}

		string code = "fld " + a1 + "\n";
		code += "fldz \n" "fcomi st(0), st(1) \n" "fstp st(0) \n";
		code += "fld " + a1 + "\n";
		code += "frndint \n";
		code += "jbe _str_pos" + st_c + "\n";
		code += "jmp _str_neg" + st_c + "\n";
		code += "_str_pos" + st_c + ":\n";
		code += "fcomi st(0), st(1) \n" "fist _str_int \n";
		code += "jbe _str_cont" + st_c + "\n";
		code += "dec _str_int \n";
		code += "jmp _str_cont" + st_c + "\n";
		code += "_str_neg" + st_c + ":\n";
		code += "fcomi st(0), st(1) \n" "fist _str_int \n";
		code += "jnb _str_cont" + st_c + "\n";
		code += "inc _str_int \n";
		code += "_str_cont" + st_c + ":\n";
		code += R"(fstp st(0)
fild _str_int
fsub st(0), st(1)
fabs
fild _str_pow
fmul st(0), st(1)
fistp _str_fract
fstp st(0)
fstp st(0)
mov edi, offset _str_res
mov eax, _str_int
call _str
mov esi, offset _str_dot
movsb
mov eax, _str_fract
call _str
mov esi, offset _str_temp
movsb
)";
		code += "mov ecx, lengthof _str_res \n";
		code += "mov edi, offset " + tvar + "\n";
		code += "mov esi, offset _str_res \n";
		code += "loop" + st_c + ":\n";
		code += "movsb \n" "dec ecx \n" "cmp ecx, 0 \n";
		code += "je end" + st_c + "\n";
		code += "mov eax, -1 \n" "cmp [esi], eax \n";
		code += "je end" + st_c + "\n";
		code += "jmp loop" + st_c + "\n";
		code += "end" + st_c + ":\n";

		this->asm_fun_tcode += proc;
		this->asm_vars += vars;

		this->asm_vars += tvar + " db 1024 dup (0)" + "\n";
		Variable v; v.name = tvar; v.type = "str";
		rnt_var_table.push_back(v);

		this->asm_tcode += code;
		this->asm_tcode += "mov ecx, sizeof " + tvar + "\n";
		this->asm_tcode += "mov esi, offset " + tvar + "\n";

		tree.value = tvar;
		tree.node_type = Nodes::Var;
		tree.subs = {};

		return true;
	}

	return false;
}

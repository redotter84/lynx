#pragma once

#include "env.h"

const unordered_set<string> db_key = { // keywords
	"if", "else", "for", "def", "block", "end"
};
const unordered_set<string> db_stat = {
	"if", "for", "def", "block"
};
const unordered_set<string> db_lit = {
	"nil"
};
const unordered_set<string> db_oper = { // operators
	"=", "+", "-", "--", "*", "/", "%", "^", "<", ">", "<=", ">=", "==", "/=",
	"and", "or", "not", "@", "++", ",", ".", "let", "const", "arg", "return"
};
const unordered_set<string> db_arith_oper = {
	"+", "-", "--", "*", "/", "%", "^"
};
const unordered_set<string> db_comp_oper = {
	"<", ">", "<=", ">=", "==", "/="
};
const unordered_set<string> db_logic_oper = {
	"and", "or", "not"
};
const unordered_set<string> db_let_oper = {
	"let", "const", "arg"
};
const unordered_set<string> db_arr_oper = {
	"@", "++"
};
const unordered_set<string> db_un_oper{
	"not", "return", "--"
};
const unordered_map<string, int> db_oper_prec = {
	make_pair("^", 11),
	make_pair("--", 11),
	make_pair("*", 10),
	make_pair("/", 10),
	make_pair("%", 10),
	make_pair("+", 9),
	make_pair("-", 9),
	make_pair("@", 8),
	make_pair("<", 7),
	make_pair(">", 7),
	make_pair("<=", 7),
	make_pair(">=", 7),
	make_pair("==", 6),
	make_pair("/=", 6),
	make_pair("not", 5),
	make_pair("and", 4),
	make_pair("or", 3),
	make_pair("let", 1),
	make_pair("const", 1),
	make_pair("arg", 1),
	make_pair(",", 0),
	make_pair("++", -1),
	make_pair("=", -2),
	make_pair("return", -3)
};
const int db_max_prec = 11;
const unordered_set<string> db_oper_right = {
	"=", "^", ","
};
const unordered_set<string> db_vars = {
	"num", "str", "arr"
};
const vector<string> db_funs = {
	"print", "input", "sin", "cos", "tan", "log", "round", "str"
};

#pragma once

#include "database.h"
#include "env.h"
#include "tools.h"

string str_token(const Tokens t); // string representing of token
string str_node(const Nodes node_type);
string erase_space(const string code); // erase spaces from begin and end of line
bool is_num(const string input); // check if string is number
bool is_name(const string input); // check if string is right name
vector<string> load_code(const string filename); // load code from source file
bool is_token_fun(const vector<Token>::const_iterator& token, const vector<Token>& tokens);
vector<Token> dijkstra(const vector<Token>& tokens);
vector<Token> go_throw(const vector<Token>& code, const string end_str, const size_t beg = 1);
int find_end(const vector<Token>& tokens, const string stat);
void replace_all(string& str, const string& s1, const string& s2);

template <typename T>
void merge(vector<T>& v1, const vector<T>& v2)
{
	v1.insert(v1.cend(), v2.cbegin(), v2.cend());
}

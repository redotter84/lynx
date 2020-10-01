#pragma once

#pragma warning(push, 0)
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#pragma warning(pop)

using std::count;
using std::cout;
using std::distance;
using std::endl;
using std::find;
using std::for_each;
using std::ifstream;
using std::make_pair;
using std::make_shared;
using std::multiset;
using std::pair;
using std::reverse;
using std::shared_ptr;
using std::stack;
using std::stoi;
using std::string;
using std::to_string;
using std::transform;
using std::unordered_map;
using std::unordered_set;
using std::vector;

enum class Tokens // enum for tokens for scaner
{
	Name, Key, Lit, Oper, Br, Sep, Fun, FunName, Unknown, Macro
};

using Token = pair<Tokens, string>;

enum class Nodes {
	Program, Block, Expr, Stat, Lit, Oper, Fun, FunName, Var, Arr, Unknown
};

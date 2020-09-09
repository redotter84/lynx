#include "tree.h"

Nodes SyntaxTree::id_type(Token token) const
{
	if (token.first == Tokens::Macro)
	{
		if (token.second == "program")
		{
			return Nodes::Program;
		}
		else if (token.second == "block" || token.second == "if-then" ||
			token.second == "if-else" || token.second == "for-do" ||
			token.second == "def-do")
		{
			return Nodes::Block;
		}
		else if (token.second == "expr" || token.second == "if-cond" ||
			token.second == "for-cond" || token.second == "for-init" ||
			token.second == "for-incr" || token.second == "def-name" ||
			token.second == "def-type" || token.second == "def-arg")
		{
			return Nodes::Expr;
		}
		else if (token.second == "if" || token.second == "for" ||
			token.second == "def")
		{
			return Nodes::Stat;
		}
		else
		{
			return Nodes::Unknown;
		}
	}
	else if (token.first == Tokens::Lit)
	{
		return Nodes::Lit;
	}
	else if (token.first == Tokens::Oper)
	{
		return Nodes::Oper;
	}
	else if (token.first == Tokens::Name)
	{
		return Nodes::Var;
	}
	else if (token.first == Tokens::Fun)
	{
		return Nodes::Fun;
	}
	else if (token.first == Tokens::FunName)
	{
		return Nodes::FunName;
	}
	else if (token.first == Tokens::Br && token.second == "[")
	{
		return Nodes::Arr;
	}
	else
	{
		return Nodes::Unknown;
	}
}

string SyntaxTree::view(const string n) const
{
	string res = n + value + " (" + str_node(node_type) + ")";
	for (const auto& iter : this->subs)
	{
		if (iter != nullptr)
		{
			res += iter->view(n + "  ");
		}
	}
	return res;
}

SyntaxTree::SyntaxTree(){}

SyntaxTree::SyntaxTree(Token token)
{
	this->value = token.second;
	this->node_type = this->id_type(token);
}

SyntaxTree::operator string() const
{
	return this->view("\n");
}

SyntaxTree create_tree(vector<Token>* tokens)
{
	if (tokens->empty())
	{
		return SyntaxTree(Token(Tokens::Macro, ""));
	}
	SyntaxTree tree = SyntaxTree((*tokens)[0]);
	tokens->erase(tokens->cbegin());

	if (tree.node_type == Nodes::Program || tree.node_type == Nodes::Block ||
		tree.node_type == Nodes::Expr || tree.node_type == Nodes::Stat)
	{
		const int end_pos = find_end(*tokens, tree.value);
		vector<Token> contain(tokens->cbegin(), tokens->cbegin() + end_pos);

		while (!contain.empty())
		{
			auto sub = make_shared<SyntaxTree>(create_tree(&contain));
			tree.subs.push_back(sub);
		}
		tokens->erase(tokens->cbegin(), tokens->cbegin() + end_pos + 1);
	}
	else if (tree.node_type == Nodes::Oper)
	{
		if (db_un_oper.find(tree.value) != db_un_oper.cend())
		{
			auto sub = make_shared<SyntaxTree>(create_tree(tokens));
			tree.subs.push_back(sub);
		}
		else
		{
			auto right = make_shared<SyntaxTree>(create_tree(tokens));
			auto left = make_shared<SyntaxTree>(create_tree(tokens));
			tree.subs.push_back(left);
			tree.subs.push_back(right);
		}
	}
	else if (tree.node_type == Nodes::Fun)
	{
		auto sub = make_shared<SyntaxTree>(create_tree(tokens));
		tree.subs.push_back(sub);
	}
	else if (tree.node_type == Nodes::Lit || tree.node_type == Nodes::Var || 
		tree.node_type == Nodes::FunName)
	{
		tree.subs = {};
	}
	else if (tree.node_type == Nodes::Arr)
	{
		auto sub = make_shared<SyntaxTree>(create_tree(tokens));
		tree.subs.push_back(sub);
	}
	else
	{
		tree.subs = {};
	}
	return tree;
}

string arr_str(const SyntaxTree& tree)
{
	string res;
	if (tree.value != ",")
	{
		res = tree.value;
		return res;
	}
	res += tree.subs[0]->value;
	res += ", ";
	res += arr_str(*tree.subs[1]);
	return res;
}

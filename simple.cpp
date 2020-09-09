#include "simple.h"

vector<Variable> rnt_var_table = {};
vector<Function> rnt_fun_table = {};

bool Variable::operator < (const Variable& v) const
{
	return this->name < v.name;
}

bool Variable::operator == (const Variable& v) const
{
	return this->name == v.name;
}

bool Function::operator < (const Function& f) const
{
	return this->name < f.name;
}

bool Function::operator == (const Function& f) const
{
	return this->name == f.name;
}

void SimpleTree::get_vars(const SyntaxTree& tree, const string path, const string curp) const
{
	if (path == "0")
	{
		for (const auto v : db_vars)
		{
			Variable var;
			var.name = v;
			var.path = "0.0.0";
			rnt_var_table.push_back(var);
		}
	}
	if (tree.value == "let" || tree.value == "const" || tree.value == "arg")
	{
		Variable var;
		var.name = tree.subs[0]->value;
		var.value = "";
		var.type = tree.subs[1]->value;
		if (curp == "")
		{
			var.path = path;
		}
		else
		{
			var.path = curp;
		}
		if (tree.value == "const")
		{
			var.is_const = true;
			var.is_def = false;
		}
		else
		{
			var.is_const = false;
			var.is_def = false;
		}
		rnt_var_table.push_back(var);
	}
	for (size_t i = 0; i != tree.subs.size(); i++)
	{
		if (i == 1 && tree.value == "def")
		{
			this->get_vars(*tree.subs[1], path + ".1", path + ".0");
		}
		else if (i == 0 && tree.value == "for")
		{
			this->get_vars(*tree.subs[0], path + ".0", path + ".0");
		}
		else if (tree.node_type == Nodes::Oper)
		{
			this->get_vars(*tree.subs[i], path, curp);
		}
		else
		{
			this->get_vars(*tree.subs[i], path + "." + to_string(i), curp);
		}
	}
}

void SimpleTree::get_funs(const SyntaxTree& tree, const string path) const
{
	if (path == "0")
	{
		for (const auto f : db_funs)
		{
			Function fun;
			fun.name = f;
			fun.path = "0.0.0";
			rnt_fun_table.push_back(fun);
		}
	}
	if (tree.value == "def")
	{
		Function fun;
		fun.name = tree.subs[0]->subs[0]->value;
		fun.type = tree.subs[2]->subs[0]->value;
		fun.path = path;
		rnt_fun_table.push_back(fun);
	}
	for (size_t i = 0; i != tree.subs.size(); i++)
	{
		this->get_funs(*tree.subs[i], path + "." + to_string(i));
	}
}

bool SimpleTree::good_path(const string p1, const string p2) const // p1 - current, p2 - var

{
	if (count(p1.cbegin(), p1.cend(), '.') < count(p2.cbegin(), p2.cend(), '.'))
	{
		return false;
	}

	string tp2 = p2.substr(0, p2.find_last_of('.'));
	tp2 = tp2.substr(0, tp2.find_last_of('.'));
	string tp1 = p1.substr(0, tp2.length());

	if (tp1 != tp2)
	{
		return false;
	}

	tp1 = p1.substr(tp1.length() + 1);
	tp2 = p2.substr(tp2.length() + 1);

	if (tp1.length() < tp2.length())
	{
		return false;
	}
	if (std::stoi(tp1) < std::stoi(tp2))
	{
		return false;
	}

	return true;
}

void SimpleTree::simple_tree(SyntaxTree& tree, const string path) const
{
	for (size_t i = 0; i != tree.subs.size(); i++)
	{
		this->simple_tree(*tree.subs[i], path + "." + to_string(i));
	}
	if (tree.node_type == Nodes::Oper)
	{
		if (tree.subs.size() == 2 &&
			tree.subs[0]->node_type == Nodes::Lit && is_num(tree.subs[0]->value) &&
			tree.subs[1]->node_type == Nodes::Lit && is_num(tree.subs[1]->value))
		{
			auto lt = *tree.subs[0];
			auto rt = *tree.subs[1];
			if (tree.value == "+")
			{
				tree.value = to_string(stoi(lt.value) + stoi(rt.value));
				if (tree.value.find('.') == string::npos)
				{
					tree.value += ".0";
				}
				tree.node_type = Nodes::Lit;
				tree.subs = {};
			}
			if (tree.value == "-")
			{
				tree.value = to_string(stoi(lt.value) - stoi(rt.value));
				if (tree.value.find('.') == string::npos)
				{
					tree.value += ".0";
				}
				tree.node_type = Nodes::Lit;
				tree.subs = {};
			}
			if (tree.value == "*")
			{
				tree.value = to_string(stoi(lt.value) * stoi(rt.value));
				if (tree.value.find('.') == string::npos)
				{
					tree.value += ".0";
				}
				tree.node_type = Nodes::Lit;
				tree.subs = {};
			}
			if (tree.value == "/")
			{
				tree.value = to_string(stoi(lt.value) / stoi(rt.value));
				if (tree.value.find('.') == string::npos)
				{
					tree.value += ".0";
				}
				tree.node_type = Nodes::Lit;
				tree.subs = {};
			}
		}

		if (tree.value == "*")
		{
			auto lt = *tree.subs[0];
			auto rt = *tree.subs[1];
			if (lt.value == "0.0" || rt.value == "0.0")
			{
				tree.value = "0.0";
				tree.node_type = Nodes::Lit;
				tree.subs = {};
			}
		}
	}
	else if (tree.node_type == Nodes::Lit)
	{
		if (tree.value[0] == '"')
		{
			replace_all(tree.value, "\\\\", "\\");
			replace_all(tree.value, "\"\"", "\\0");

			size_t f = tree.value.find("\\n");
			while (f != string::npos)
			{
				if (tree.value == "\"\\n\"")
				{
					tree.value = "!str10";
				}
				else
				{
					tree.value.replace(f, 2, "\", 10, \"");
				}
				f = tree.value.find("\\n");
			}

			f = tree.value.find("\\0");
			while (f != string::npos)
			{
				if (tree.value == "\"\\0\"")
				{
					tree.value = "!str0";
				}
				else
				{
					tree.value.replace(f, 2, "\", 0, \"");
				}
				f = tree.value.find("\\0");
			}

			replace_all(tree.value, "\"\"", "");
			if (tree.value.substr(0, 2) == ", ")
			{
				tree.value.erase(0, 2);
			}
			if (tree.value.substr(tree.value.length() - 2, 1) == " \"")
			{
				tree.value.erase(tree.value.length() - 2);
			}
			if (tree.value == "\", 0, \"")
			{
				tree.value = "!str0";
			}
		}
	}
	else if (tree.node_type == Nodes::Var)
	{
		int good = -1;

		Variable var;
		var.name = tree.value;

		for (auto i = rnt_var_table.cbegin(); i != rnt_var_table.cend(); i++)
		{
			if (*i == var)
			{
				if (this->good_path(path, i->path))
				{
					good = distance(rnt_var_table.cbegin(), i);
					break;
				}
			}
		}
		if (good == -1)
		{
			throw "unknown variable";
		}
		tree.value = "var" + to_string(good);
	}
	else if (tree.node_type == Nodes::Fun || tree.node_type == Nodes::FunName)
	{
		int good = -1;

		Function fun;
		fun.name = tree.value;

		for (auto i = rnt_fun_table.cbegin(); i != rnt_fun_table.cend(); i++)
		{
			if (*i == fun)
			{
				good = distance(rnt_fun_table.cbegin(), i);
				break;
			}
		}
		if (good == -1)
		{
			throw "unknown function";
		}
		tree.value = "fun" + to_string(good);
	}
}

SimpleTree::SimpleTree(SyntaxTree& tree)
{
	this->get_funs(tree);
	this->get_vars(tree);
	this->simple_tree(tree);
}

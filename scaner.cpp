#include "scaner.h"

string Scaner::compress_code(const vector<string>& c) const // join lines to one string
{
	if (c.empty())
	{
		return "";
	}

	string res;
	for (const auto& iter : c)
	{
		size_t com_pos = iter.length();
		bool instr = false;
		for (size_t i = 0; i != iter.length(); i++)
		{
			if (iter[i] == '"')
			{
				instr = !instr;
			}
			else if (!instr && iter[i] == '#')
			{
				com_pos = i;
				break;
			}
		}
		res += iter.substr(0, com_pos);
		if (iter[iter.length() - 1] == '\\')
		{
			if (!res.empty())
			{
				res.erase(res.length() - 1);
			}
		}
		else if (iter[iter.length() - 1] != ';' && iter[iter.length() - 1] != ':')
		{
			res += "; ";
		}
		else
		{
			res += " ";
		}
	}
	res.erase(res.length() - 1);
	return res;
}

Tokens Scaner::id_token(string& token) const // recognize which token is for string
{
	if (db_oper.find(token) != db_oper.cend()) // operators
	{
		return Tokens::Oper;
	}
	else if (db_key.find(token) != db_key.cend()) // keyword
	{
		return Tokens::Key;
	}
	else if (is_num(token))
	{
		if (token.find('.') == string::npos)
		{
			token += ".0";
		}
		return Tokens::Lit;
	}
	else if (token[0] == '"' ||
		db_lit.find(token) != db_lit.cend()) // string or number or true/false/nil
	{
		return Tokens::Lit;
	}
	else if (token == ":" || token == ";") // separator
	{
		return Tokens::Sep;
	}
	else if (token == "(" || token == ")" || token == "[" || token == "]" ||
		token == "{" || token == "}") // bracket
	{
		return Tokens::Br;
	}
	else if (token == "")
	{
		return Tokens::Unknown;
	}
	else
	{
		return Tokens::Name;
	}
}

Scaner::Scaner(const vector<string>& c) // constructor
{
	_code = compress_code(c);
}

vector<Token> Scaner::get_tokens() const // replace code to tokens
{
	vector<Token> res;
	string current;
	bool instr = false;

	for (size_t i = 0; i != _code.length(); i++)
	{
		if (current == "-" && !isdigit(_code[i]))
		{
			if (!res.empty())
			{
				Tokens top = res.at(res.size() - 1).first;
				if (top != Tokens::Lit && top != Tokens::Name)
				{
					res.push_back(Token(Tokens::Oper, "--"));
				}
				else
				{
					res.push_back(Token(Tokens::Oper, "-"));
				}
			}
			else
			{
				res.push_back(Token(Tokens::Oper, "--"));
			}
			current = "";
		}
		if (isalnum(_code[i]) || _code[i] == '_') // number or name
		{
			current += _code[i];
		}
		else if (_code[i] == '-' && current.empty()) // negative number
		{
			current = "-";
		}
		else if (_code[i] == '.' && (current.empty() || is_num(current))) // fraction
		{
			current += '.';
		}
		else if (_code[i] == '"') // string
		{
			string temp = _code;
			replace_all(temp, "\\", "b");
			instr = !instr || (i != 0 && temp[i - 1] == '\\');
			current += '"';
		}
		else if (instr) // string
		{
			current += _code[i];
		}
		else if (_code[i] == '(' && _code[i + 1] == ')')
		{
			res.push_back(Token(this->id_token(current), current));
			res.push_back(Token(Tokens::Br, "("));
			res.push_back(Token(Tokens::Lit, "nil"));
			res.push_back(Token(Tokens::Br, ")"));
			current = "";
			++i;
		}
		else if (_code[i] == '[' && _code[i + 1] == ']')
		{
			res.push_back(Token(this->id_token(current), current));
			res.push_back(Token(Tokens::Br, "["));
			res.push_back(Token(Tokens::Lit, "nil"));
			res.push_back(Token(Tokens::Br, "]"));
			current = "";
			++i;
		}
		else if (_code[i] == '{' && _code[i + 1] == '}')
		{
			res.push_back(Token(this->id_token(current), current));
			res.push_back(Token(Tokens::Br, "{"));
			res.push_back(Token(Tokens::Lit, "nil"));
			res.push_back(Token(Tokens::Br, "}"));
			current = "";
			++i;
		}
		else
		{
			if (!current.empty() && current != " " && current != "\t") // push previous
			{
				res.push_back(Token(this->id_token(current), current));
			}

			current = _code[i];
			if (this->id_token(current) == Tokens::Oper && i != _code.length() - 1) // ==, /=, etc.
			{
				string temp = current + _code[i + 1];
				if (this->id_token(temp) == Tokens::Oper)
				{
					current = temp;
					i++;
				}
			}

			if (!current.empty() && current != " " && current != "\t")
			{
				res.push_back(Token(this->id_token(current), current));
			}
			current = "";
		}
	}
	const Token t = Token(this->id_token(current), current);
	if (t.first != Tokens::Unknown)
	{
		res.push_back(t);
	}

	return res;
}

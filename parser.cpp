#pragma once

#include "stdafx.h"

#include "parser.h"

vector<Token> Parser::sugar_while(const vector<Token>& tokens, 
	vector<Token>::const_iterator& iter) const
{
	vector<Token> res;

	res.push_back(Token(Tokens::Key, "for"));
	res.push_back(Token(Tokens::Lit, "nil"));
	res.push_back(Token(Tokens::Sep, ";"));

	const vector<Token> cond_code = go_throw(vector<Token>(iter + 1,
		tokens.cend()), ":");
	merge(res, sugar(cond_code));
	res.push_back(Token(Tokens::Sep, ";"));
	res.push_back(Token(Tokens::Lit, "nil"));
	res.push_back(Token(Tokens::Sep, ":"));

	const vector<Token> do_code = go_throw(vector<Token>(iter + int(cond_code.size()) + 2,
		tokens.cend()), "end");
	merge(res, sugar(do_code));
	res.push_back(Token(Tokens::Key, "end"));
	res.push_back(Token(Tokens::Sep, ";"));

	iter += int(cond_code.size() + do_code.size() + 3);

	return res;
}

vector<Token> Parser::sugar_elif(const vector<Token>& tokens, 
	vector<Token>::const_iterator& iter) const 
{
	vector<Token> res;

	res.push_back(Token(Tokens::Key, "else"));
	res.push_back(Token(Tokens::Sep, ":"));
	res.push_back(Token(Tokens::Key, "if"));

	const vector<Token> cond_code = go_throw(vector<Token>(iter + 1, tokens.cend()), ":");
	merge(res, sugar(cond_code));
	res.push_back(Token(Tokens::Sep, ":"));

	vector<Token> t_code = go_throw(vector<Token>(iter + int(cond_code.size()) + 2,
		tokens.cend()), "elif");
	if (t_code.size() != 0)
	{
		const vector<Token> then_code(iter + int(cond_code.size() + 2), tokens.cend());
		merge(res, sugar(then_code));

		res.push_back(Token(Tokens::Key, "end"));
		res.push_back(Token(Tokens::Sep, ";"));

		iter += int(cond_code.size() + then_code.size() + 1);
		return res;
	}
	else
	{
		const vector<Token> then_code = go_throw(vector<Token>(iter + int(cond_code.size()) + 2,
			tokens.cend()), "else");
		merge(res, sugar(then_code));

		res.push_back(Token(Tokens::Key, "else"));
		res.push_back(Token(Tokens::Sep, ":"));

		const vector<Token> else_code = go_throw(vector<Token>(
			iter + int(cond_code.size()) + 2 + int(then_code.size()) + 2, tokens.cend()), "end");
		merge(res, sugar(else_code));

		res.push_back(Token(Tokens::Key, "end"));
		res.push_back(Token(Tokens::Sep, ";"));
	
		iter += int(cond_code.size() + then_code.size() + 2 + else_code.size() + 1);

		return res;
	}
}

vector<Token> Parser::sugar(const vector<Token>& tokens) const 
{
	vector<Token> res;
	for (auto iter = tokens.cbegin(); iter != tokens.cend(); iter++)
	{
		if (iter->second == "while")
		{
			merge(res, this->sugar_while(tokens, iter));
		}
		else if (iter->second == "elif")
		{
			merge(res, this->sugar_elif(tokens, iter));
		}
		else
		{
			res.push_back(*iter);
		}
	}
	return res;
}

vector<Token> Parser::parse_if(const vector<Token>& code) const
{
	vector<Token> res(2);
	res[0] = Token(Tokens::Macro, "if");

	res[1] = Token(Tokens::Macro, "if-cond");
	const vector<Token> cond_code = go_throw(code, ":");
	const auto cond_stat = this->get_expr(cond_code);
	merge(res, cond_stat);
	int count = int(cond_code.size());
	res.push_back(Token(Tokens::Macro, "end-if-cond"));

	res.push_back(Token(Tokens::Macro, "if-then"));
	const vector<Token> t_then_code(code.cbegin() + count, code.cend());
	const vector<Token> then_code = go_throw(t_then_code, "else");
	const auto then_stat = this->get_expr(then_code);
	merge(res, then_stat);
	count += then_code.size();
	res.push_back(Token(Tokens::Macro, "end-if-then"));

	res.push_back(Token(Tokens::Macro, "if-else"));
	const vector<Token> else_code(code.cbegin() + count + 2, code.cend());
	const auto else_stat = get_expr(else_code);
	merge(res, else_stat);
	res.push_back(Token(Tokens::Macro, "end-if-else"));

	res.push_back(Token(Tokens::Macro, "end-if"));
	return res;
}

vector<Token> Parser::parse_for(const vector<Token>& code) const
{
	vector<Token> res(2);
	res[0] = Token(Tokens::Macro, "for");

	res[1] = Token(Tokens::Macro, "for-init");
	const auto first = std::find(code.cbegin(), code.cend(), Token(Tokens::Sep, ";"));
	const vector<Token> init_code(code.cbegin(), first + 1);
	const auto init_stat = this->get_expr(init_code);
	merge(res, init_stat);
	
	res.push_back(Token(Tokens::Macro, "end-for-init"));

	res.push_back(Token(Tokens::Macro, "for-cond"));
	const auto second = std::find(first + 1, code.cend(), Token(Tokens::Sep, ";"));
	const vector<Token> cond_code(first + 1, second + 1);
	const auto cond_stat = this->get_expr(cond_code);
	merge(res, cond_stat);
	res.push_back(Token(Tokens::Macro, "end-for-cond"));

	res.push_back(Token(Tokens::Macro, "for-incr"));
	const vector<Token> t_incr_code(second + 1, code.cend());
	const vector<Token> incr_code = go_throw(t_incr_code, ":");
	const auto incr_stat = this->get_expr(incr_code);
	merge(res, incr_stat);
	const int count = int(init_code.size() + cond_code.size() + incr_code.size());
	res.push_back(Token(Tokens::Macro, "end-for-incr"));

	res.push_back(Token(Tokens::Macro, "for-do"));
	const vector<Token> do_code(code.cbegin() + count, code.cend());
	const auto do_stat = this->get_expr(do_code);
	merge(res, do_stat);
	res.push_back(Token(Tokens::Macro, "end-for-do"));

	res.push_back(Token(Tokens::Macro, "end-for"));
	return res;
}

vector<Token> Parser::parse_def(const vector<Token>& code) const
{
	vector<Token> res(2);

	res[0] = Token(Tokens::Macro, "def");

	res[1] = Token(Tokens::Macro, "def-name");
	const auto opar = find(code.cbegin(), code.cend(), Token(Tokens::Br, "("));
	const auto cpar = find(code.cbegin(), code.cend(), Token(Tokens::Br, ")"));
	res.push_back(Token(Tokens::FunName, (opar - 1)->second));
	res.push_back(Token(Tokens::Macro, "end-def-name"));

	res.push_back(Token(Token(Tokens::Macro, "def-arg")));
	const vector<Token> arg_code(opar + 1, cpar);
	const auto arg_stat = this->get_expr(arg_code);
	merge(res, arg_stat);
	res.push_back(Token(Tokens::Macro, "end-def-arg"));

	res.push_back(Token(Token(Tokens::Macro, "def-type")));
	res.push_back(*(cpar + 1));
	res.push_back(Token(Tokens::Macro, "end-def-type"));

	res.push_back(Token(Token(Tokens::Macro, "def-do")));
	const vector<Token> do_code(cpar + 3, code.cend());
	const auto do_stat = this->get_expr(do_code);
	merge(res, do_stat);
	res.push_back(Token(Tokens::Macro, "end-def-do"));

	res.push_back(Token(Tokens::Macro, "end-def"));
	return res;
}

vector<Token> Parser::parse_block(const vector<Token>& code) const
{
	vector<Token> res(1);
	res[0] = Token(Tokens::Macro, "block");

	const auto do_stat = this->get_expr(code);
	merge(res, do_stat);

	res.push_back(Token(Token(Tokens::Macro, "end-block")));
	return res;
}

vector<Token> Parser::parse_stat(const vector<Token>& code) const
{
	const vector<Token> body(code.cbegin() + 1, code.cend());
	if (code[0].second == "if")
	{
		return this->parse_if(body);
	}
	else if (code[0].second == "for")
	{
		return this->parse_for(body);
	}
	else if (code[0].second == "def")
	{
		return this->parse_def(body);
	}
	else if (code[0].second == "block")
	{
		return this->parse_block(body);
	}
	return{};
}

vector<Token> Parser::get_expr(const vector<Token>& code) const
{
	vector<Token> res;
	vector<Token> temp;
	for (auto iter = code.cbegin(); iter != code.cend(); iter++)
	{
		if (iter->first == Tokens::Sep)
		{
			if (!temp.empty())
			{
				res.push_back(Token(Tokens::Macro, "expr"));
				merge(res, dijkstra(temp));
				res.push_back(Token(Tokens::Macro, "end-expr"));
				temp = {};
			}
		}
		else if (iter->first == Tokens::Key)
		{
			if (!temp.empty())
			{
				res.push_back(Token(Tokens::Macro, "expr"));
				merge(res, dijkstra(temp));
				res.push_back(Token(Tokens::Macro, "end-expr"));
				temp = {};
			}
			if (db_stat.find(iter->second) != db_stat.cend())
			{
				const vector<Token> stat = go_throw(vector<Token>(iter, code.cend()), "end", 0);
				iter += int(stat.size());
				const auto ext_stat = this->parse_stat(stat);
				for (const auto& expr : ext_stat)
				{
					res.push_back(expr);
				}
			}
			else
			{
				res.push_back(*iter);
			}
		}
		else
		{
			temp.push_back(*iter);
		}
	}
	if (!temp.empty())
	{
		res.push_back({ Token(Tokens::Macro, "expr") });
		merge(res, dijkstra(temp));
		res.push_back({ Token(Tokens::Macro, "end-expr") });
	}
	return res;
}

vector<Token> Parser::get_struct() const
{
	vector<Token> res(1);
	res[0] = Token(Tokens::Macro, "program");
	auto prog = this->get_expr(this->sugar(_code));
	merge(res, prog);
	res.push_back({ Token(Tokens::Macro, "end-program") });
	return res;
}

Parser::Parser(const vector<Token>& c)
{
	_code = c;
}

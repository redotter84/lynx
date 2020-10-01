#include "tools.h"

string str_token(const Tokens t) // string representing of token
{
	switch (t) {
	case Tokens::Name:
		return "Name";
	case Tokens::Key:
		return "Key";
	case Tokens::Lit:
		return "Lit";
	case Tokens::Oper:
		return "Oper";
	case Tokens::Sep:
		return "Sep";
	case Tokens::Br:
		return "Br";
	case Tokens::Fun:
		return "Fun";
	case Tokens::FunName:
		return "FunName";
	case Tokens::Macro:
		return "Macro";
	case Tokens::Unknown:
		return "UNKNOWN";
	default:
		return "MAGIC";
	}
}

string str_node(const Nodes node_type) {
	switch (node_type) {
	case Nodes::Program:
		return "Program";
	case Nodes::Block:
		return "Block";
	case Nodes::Expr:
		return "Expr";
	case Nodes::Stat:
		return "Stat";
	case Nodes::Lit:
		return "Lit";
	case Nodes::Oper:
		return "Oper";
	case Nodes::Fun:
		return "Fun";
	case Nodes::FunName:
		return "FunName";
	case Nodes::Var:
		return "Var";
	case Nodes::Arr:
		return "Arr";
	case Nodes::Unknown:
		return "UNKNOWN";
	default:
		return "MAGIC";
	}
}

string erase_space(const string code) // erase spaces from begin and end of line
{
	string res = code;

	for (size_t i = 0; i != res.length(); i++) // from begin
	{
		if (res[i] != ' ' && res[i] != '\t') {
			break;
		}
		res.erase(i, 1);
		i--;
	}

	for (size_t i = res.length() - 1; i != -1; i--) // from end
	{
		if (res[i] != ' ' && res[i] != '\t') {
			break;
		}
		res.erase(i, 1);
		i++;
	}

	return res;
}

bool is_num(const string input) // check if string is number
{
	string num = input;
	if (num.empty()) {
		return false;
	}

	if (input[0] == '+' || input[0] == '-') // if positive/negative
	{
		num = input.substr(1);
	}

	bool point = false; // if fraction

	for (size_t i = 0; i != num.length(); i++) {
		if (num[i] == '.') {
			if (point) {
				return false;
			}
			point = true;
		} else if (!isdigit(num[i])) {
			return false;
		}
	}

	return !num.empty(); // `-` is not a number
}

vector<string> load_code(const string filename) // load code from source file
{
	vector<string> lines;

	ifstream file(filename);
	if (file.is_open()) {
		while (file.good()) {
			string temp;
			getline(file, temp);
			temp = erase_space(temp);

			if (!temp.empty()) {
				lines.push_back(temp);
			}
		}
	}
	file.close();

	return lines;
}

bool is_token_fun(const vector<Token>::const_iterator& token, const vector<Token>& tokens) {
	return (token != tokens.cend() - 1 &&
		token->first == Tokens::Name && (token + 1)->second == "(");
}

vector<Token> dijkstra(const vector<Token>& tokens) {
	vector<Token> res;
	stack<Token> st;

	for (auto iter = tokens.cbegin(); iter != tokens.cend(); iter++) {
		if (is_token_fun(iter, tokens)) {
			st.push(Token(Tokens::Fun, iter->second));
		} else if (iter->first == Tokens::Lit || iter->first == Tokens::Name ||
			iter->first == Tokens::FunName) {
			res.push_back(*iter);
		} else if (iter->first == Tokens::Oper) {
			const int iter_prec = db_oper_prec.at(iter->second);
			int top_prec = 0;
			const bool is_left = db_oper_right.find(iter->second) == db_oper_right.cend();

			if (!st.empty() && st.top().second != "(") {
				top_prec = (st.top().first == Tokens::Fun) ? db_max_prec :
					db_oper_prec.at(st.top().second);
			}

			while (!st.empty() && (st.top().second != "(") && (
				(is_left && iter_prec <= top_prec) ||
				(!is_left && iter_prec < top_prec))) {
				res.push_back(st.top());
				st.pop();
				if (!st.empty() && st.top().second != "(") {
					top_prec = (st.top().first == Tokens::Fun) ? db_max_prec :
						db_oper_prec.at(st.top().second);
				}
			}
			st.push(*iter);
		} else if (iter->second == "(") {
			st.push(*iter);
		} else if (iter->second == ")") {
			while (!st.empty() && st.top().second != "(") {
				res.push_back(st.top());
				st.pop();
			}
			st.pop();
		}
	}
	while (!st.empty()) {
		res.push_back(st.top());
		st.pop();
	}
	reverse(res.begin(), res.end());
	return res;
}

vector<Token> go_throw(const vector<Token>& code, const string end_str, const size_t beg) {
	auto end = code.cbegin();
	size_t count = beg;

	for (auto iter = code.cbegin(); iter != code.cend(); iter++) {
		if (db_stat.find(iter->second) != db_stat.cend() ||
			(end_str == ":" && iter->second == "else")) {
			++count;
		} else if (iter->second == end_str || iter->second == "end") {
			--count;
		}
		if (count == 0) {
			end = iter;
			break;
		}
	}
	vector<Token> res(code.cbegin(), end);
	return res;
}

int find_end(const vector<Token>& tokens, const string stat) {
	int res = 0;
	size_t count = 1;
	for (size_t i = 0; i != tokens.size(); i++) {
		if (tokens[i].second == stat) {
			++count;
		} else if (tokens[i].second == "end-" + stat) {
			--count;
		}
		if (count == 0) {
			res = (int)i;
			break;
		}
	}
	return res;
}

void replace_all(string& str, const string& s1, const string& s2) {
	size_t f = str.find(s1);
	while (f != string::npos) {
		str.replace(f, s1.length(), s2);
		f = str.find(s1, f + s2.length());
	}
}
#include "asm.h"
#include "env.h"
#include "scaner.h"
#include "parser.h"
#include "tools.h"
#include "tree.h"
#include "simple.h"

#include <windows.h>
#include <chrono>
#include <thread>

int main(const int argc, const char* argv[]) // main
{
	try {
		string filename = "code\\hello.lnx";
		bool exec = false;
		bool disp = false;
		string asm_fn = "code\\hello.asm";
		string exe_fn = "code\\hello.exe";
		if (argc > 2) {
			filename = argv[1];
			for (size_t i = 2; i != size_t(argc); i++) {
				if (string(argv[i]) == "-ex") {
					exec = true;
				} else if (string(argv[i]) == "-d") {
					disp = true;
				} else if (string(argv[i]) == "-asm") {
					asm_fn = argv[++i];
				} else if (string(argv[i]) == "-out") {
					exe_fn = argv[++i];
				}
			}
		}

		const vector<string> lines = load_code(filename);

		if (disp) {
			cout << "code" << endl << endl;
			for_each(lines.cbegin(), lines.cend(), [](const auto& el) {cout << el << endl; });
			getchar();
		}

		const Scaner scaner = Scaner(lines);
		const vector<Token> pre_tokens = scaner.get_tokens();

		if (disp) {
			cout << "scaner result" << endl << endl;
			for_each(pre_tokens.cbegin(), pre_tokens.cend(),
				[](const auto& el) { cout << str_token(el.first) << " " << el.second << endl; });
			getchar();
		}

		const Parser parser = Parser(pre_tokens);
		vector<Token> tokens = parser.get_struct();

		if (disp) {
			cout << "parser result" << endl << endl;
			for_each(tokens.cbegin(), tokens.cend(),
				[](const auto& el) { cout << str_token(el.first) << " " << el.second << endl; });
			getchar();
		}

		SyntaxTree tree = create_tree(&tokens);

		if (disp) {
			cout << "semantic result" << endl;
			cout << (string)tree << endl;
			getchar();
		}

		const SimpleTree stree = SimpleTree(tree);

		if (disp) {
			cout << "optimisator result" << endl;
			cout << (string)tree << endl;
			getchar();
		}

		const Asm asm_c = Asm(tree);

		if (disp) {
			cout << "assembly code" << endl << endl;
			cout << asm_c.asm_code << endl;
			getchar();
		}

		cout << "OK" << "\a" << endl << endl;

		std::ofstream asm_file(asm_fn);
		asm_file << asm_c.asm_code << endl;
		asm_file.close();
		const string asm_f = ".\\ml \"" + asm_fn + "\" /link /subsystem:console /out:\"" +
			exe_fn + "\"";
		system(asm_f.c_str());

		if (exec) {
			replace_all(exe_fn, " ", "\" \"");
			const string exe_f = exe_fn;
			system(exe_f.c_str());
			getchar();
		}
	} catch (const char* err) {
		cout << err << "\a" << endl;
		getchar();
	}

	return 0;
}

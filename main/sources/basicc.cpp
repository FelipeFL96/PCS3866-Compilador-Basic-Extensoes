#include <iostream>
#include <string>
#include <string.h>

#include "test.hpp"
#include "lexic.hpp"
#include "syntax.hpp"
#include "semantic.hpp"
#include "generation.hpp"
#include "ASCIIClassifier.hpp"
#include "LexicalAnalyser.hpp"
#include "SyntaxAnalyser.hpp"
#include "SemanticAnalyser.hpp"
#include "CodeGenerator.hpp"

using namespace std;


int main(int argc, char* argv[]) {
    std::cout << "Bem-Vindo ao Compilador BasicC!" << std::endl;

    if (argc < 2) {
        cerr << "Uso esperado: basicc <entrada.bas> [ <saída.s> ]" << endl;
        return 1;
    }

    string input_file = argv[1];
    string output_file = "out.s";

    ifstream input(input_file);
    if (!input.is_open()) {
        cerr << "\033[1;31mErro: \033[0m" << "Não foi possível abrir o arquivo '" << input_file << "' para entrada" << endl;
        exit(EXIT_FAILURE);
    }

    try {
        if (argc > 2 && 0 == strcmp(argv[2], "-A")) {
            ascii_test(input);
        }
        else if (argc > 2 && 0 == strcmp(argv[2], "-L")) {
            lex_test(input);
        }
        else if (argc > 2 && 0 == strcmp(argv[2], "-S")) {
            stx_test(input);
        }
        else {
            if (argc > 2)
                output_file = argv[2];

            semantic::SymbolTable symb_table;

            generation::CodeGenerator gen(input_file, output_file, symb_table);
            semantic::SemanticAnalyser smt(input, symb_table, gen);

            smt.run();
        }
    }
    catch (lexic::lexical_exception& e) {
        cerr << "\033[1;31mErro léxico: \033[37;1m" << input_file << "\033[0m" << e.message() << endl;
    }
    catch (syntax::syntax_exception& e) {
        cerr << "\033[1;31mErro sintático: \033[37;1m" << input_file << "\033[0m" << e.message() << endl;
    }
    catch (semantic::semantic_exception& e) {
        cerr << "\033[1;31mErro semântico: \033[37;1m" << input_file << "\033[0m" << e.message() << endl;
    }
    catch (generation::generation_exception& e) {
        cerr << "\033[1;31mErro de geração: \033[0m" << e.message() << endl;
    }

    input.close();

    return 0;
}

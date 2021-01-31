#ifndef CODE_GENERATOR_HPP
#define CODE_GENERATOR_HPP

#include <fstream>
#include <vector>
#include <utility>

#include "syntax.hpp"
#include "semantic.hpp"

namespace generation {

class CodeGenerator {
    public:
        CodeGenerator(std::string& input_file, std::string& output_file, semantic::SymbolTable& symb_table);
        ~CodeGenerator();

        void generate(syntax::Assign* assign, std::vector<syntax::Elem*> exp, int next_index);
        void generate(syntax::Read* read, std::vector<std::pair<syntax::Var*, syntax::Num*>>& read_data, int next_index);
        void generate(syntax::Data* data, std::vector<std::pair<syntax::Var*, syntax::Num*>>& read_data, int next_index);
        void generate(syntax::Goto* go, int destination);
        void generate(syntax::If* ift, std::vector<syntax::Elem*> left, std::vector<syntax::Elem*> right, int destination, int next_index);
        void generate(syntax::For* loop, std::vector<syntax::Elem*> init, std::vector<syntax::Elem*> stop,
                        std::vector<syntax::Elem*> step, int index_inside_loop, int index_outside_loop);
        void generate(syntax::Next* next);
        void generate(syntax::Def* def, std::vector<syntax::Elem*>& exp);
        void generate(syntax::Gosub* gosub, int destination);
        void generate(syntax::Return* ret);
        void generate(syntax::End* end);

        void generate_expression(std::vector<syntax::Elem*>& exp);
        void generate_header(int first_index);
        void generate_variables();

        void install_predef();
        void install_sdiv();
        void install_pow();

    private:
        std::ofstream output;
        std::string& input_file;
        semantic::SymbolTable& symb_table;

};

} // namespace generation

#endif // CODE_GENERATOR_HPP
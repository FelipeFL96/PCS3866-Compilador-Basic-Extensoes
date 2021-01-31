#ifndef SYNTAX_ANALYSER_HPP
#define SYNTAX_ANALYSER_HPP

#include <fstream>
#include <string>

#include "syntax.hpp"
#include "lexic.hpp"
#include "LexicalAnalyser.hpp"

namespace syntax {

class SyntaxAnalyser {
    public:
        SyntaxAnalyser(std::ifstream& file);

        syntax::BStatement* get_next(void);

    private:
        enum class method {REQUIRED, OPTIONAL, LOOKAHEAD};
        bool consume(lexic::type type, method m);

        syntax::Assign* parse_assign(int index, lexic::position pos);
        syntax::Read* parse_read(int index, lexic::position pos);
        syntax::Data* parse_data(int index, lexic::position pos);
        syntax::Print* parse_print(int index, lexic::position pos);
        syntax::Goto* parse_goto(int index, lexic::position pos);
        syntax::If* parse_if(int index, lexic::position pos);
        syntax::For* parse_for(int index, lexic::position pos);
        syntax::Next* parse_next(int index, lexic::position pos);
        syntax::Dim* parse_dim(int index, lexic::position pos);
        syntax::Def* parse_def(int index, lexic::position pos);
        syntax::Gosub* parse_gosub(int index, lexic::position pos);
        syntax::Return* parse_return(int index, lexic::position pos);
        syntax::Rem* parse_rem(int index, lexic::position pos);
        syntax::End* parse_end(int index, lexic::position pos);

        syntax::Exp* parse_exp(void);
        syntax::Eb* parse_eb(void);
        syntax::Operator* parse_operator(void);
        syntax::Num* parse_snum(void);
        syntax::Num* parse_num(void);
        syntax::Var* parse_var(void);
        syntax::Call* parse_call(void);
        syntax::Array* parse_array(void);
        syntax::Pitem* parse_pitem(void);

        bool file_end();


        std::ifstream& file;
        lexic::LexicalAnalyser lex;
        lexic::token tk;
        bool token_consumed = true;
};

} // namespace syntax

#endif // SYNTAX_ANALYSER_HPP

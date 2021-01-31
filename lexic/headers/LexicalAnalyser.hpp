#ifndef LEXICAL_ANALYZER_HPP
#define LEXICAL_ANALYZER_HPP

#include <fstream>
#include <string>
#include <exception>

#include "lexic.hpp"

#include "ASCIIClassifier.hpp"

namespace lexic {

class LexicalAnalyser {
    public:
        LexicalAnalyser(std::ifstream& file);
        lexic::token get_next();

    private:
        void change_analyser_state(lexic::type type);
        lexic::token extract_token();
        lexic::token read_comment();
        lexic::type categorize_token(std::string& value);


        void Lex(token& lexeme);
        void Wrd(token& lexeme);
        void Int(token& lexeme);
        void Spe(token& lexeme);
        void Strg(token& lexeme);
        void Chr(token& lexeme);
        void A(token& lexeme);
        void B(token& lexeme);
        void C(token& lexeme);
        void D(token& lexeme);

        ascii_character c;
        std::ifstream& file;
        ASCIIClassifier ac;
        lexic::state analyser_state;
};

} // namespace lexic

#endif // LEXICAL_ANALYZER_HPP
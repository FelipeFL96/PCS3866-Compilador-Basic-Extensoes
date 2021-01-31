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

        std::ifstream& file;
        ASCIIClassifier ac;
        lexic::state analyser_state;
};

} // namespace lexic

#endif // LEXICAL_ANALYZER_HPP
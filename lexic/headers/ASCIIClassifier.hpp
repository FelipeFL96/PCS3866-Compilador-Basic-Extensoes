#ifndef ASCII_CLASSIFIER_HPP
#define ASCII_CLASSIFIER_HPP

#include <fstream>
#include <string>

#define LINE_START 1
#define COLUMN_START 1

namespace lexic {

class ASCIIClassifier {
    public:
        ASCIIClassifier(std::ifstream& file);

        ascii_character get_next();
        ascii_character peek_next();

    private:
        ascii_type classify_character(char c);

        std::ifstream& file;
        int line, column;
};

} // namespace lexic

#endif //ASCII_CLASSIFIER_HPP
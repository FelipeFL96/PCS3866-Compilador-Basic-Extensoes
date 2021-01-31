#include <fstream>

#include "lexic.hpp"

#include "ASCIIClassifier.hpp"

using namespace std;
using namespace lexic;

ASCIIClassifier::ASCIIClassifier(ifstream& file): 
    file(file), line(LINE_START), column(COLUMN_START) {}

ascii_character ASCIIClassifier::get_next() {
    char c;
    ascii_type t;

    c = file.get();

    if (file.eof())
        c = EOF;

    t = classify_character(c);
    position p(line, column);

    ascii_character ac(c, t, p);

    if (c == '\n') {
        line++;
        column = COLUMN_START;
    }
    else {
        column++;
    }

    return ac;
}

ascii_character ASCIIClassifier::peek_next() {
    char c;
    ascii_type t;

    c = file.peek();

    if (file.eof())
        c = EOF;

    t = classify_character(c);
    position p(line, column);

    ascii_character ac(c, t, p);

    return ac;
}

ascii_type ASCIIClassifier::classify_character(char c) {
    if (
        c == '0' || c == '1' || c == '2' || c == '3' || c == '4' ||
        c == '5' || c == '6' || c == '7' || c == '8' || c == '9'
    )
        return ascii_type::DIGIT;
    else if ( 
        c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == 'E' ||
        c == 'F' || c == 'G' || c == 'H' || c == 'I' || c == 'J' ||
        c == 'K' || c == 'L' || c == 'M' || c == 'N' || c == 'O' ||
        c == 'P' || c == 'Q' || c == 'R' || c == 'S' || c == 'T' ||
        c == 'U' || c == 'V' || c == 'W' || c == 'X' || c == 'Y' ||
        c == 'Z' || c == 'a' || c == 'b' || c == 'c' || c == 'd' ||
        c == 'e' || c == 'f' || c == 'g' || c == 'h' || c == 'i' ||
        c == 'j' || c == 'k' || c == 'l' || c == 'm' || c == 'n' ||
        c == 'o' || c == 'p' || c == 'q' || c == 'r' || c == 's' ||
        c == 't' || c == 'u' || c == 'v' || c == 'w' || c == 'x' ||
        c == 'y' || c == 'z'
    )
        return ascii_type::LETTER;
    else if (
        c == '!' || c == '@' || c == '#' || c == '%' || /*  ̈   */
        c == '&' || c == '*' || c == '(' || c == ')' || c == '_' ||
        c == '+' || c == '-' || c == '='   /*  §  */ || c == '{' ||
        c == '[' || c == 'a' || c == '}' || c == ']' || c == 'o' ||
        c == '?' || c == '/'   /*  °  */ || c == '`' || /* ́   */
        c == '^' || c == '~' || c == '<' || c == ',' || c == '>' ||
        c == '.' || c == ':' || c == ';' || c == '|' || c == '\\'||
        c == '"'
    )
        return ascii_type::SPECIAL;
    else if (
        c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == EOF
    )
        return ascii_type::DELIMITER;
    else
        return ascii_type::UNKNOWN;
}
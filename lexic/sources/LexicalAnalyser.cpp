#include <iostream>
#include <fstream>
#include <string>

#include "lexic.hpp"
#include "ASCIIClassifier.hpp"

#include "LexicalAnalyser.hpp"

using namespace std;
using namespace lexic;

LexicalAnalyser::LexicalAnalyser(ifstream& file):
    c(' ', ascii_type::DELIMITER, position(0,0)), file(file), ac(file), analyser_state(state::NORMAL) {}

token LexicalAnalyser::get_next() {
    token t;

    if (analyser_state == state::COMMENT) {
        t = read_comment();
        t.type = type::CMT;
    }
    else {
        t = extract_token();
        t.type = categorize_token(t.value);
    }

    change_analyser_state(t.type);
    return t;
}

void LexicalAnalyser::change_analyser_state(type type) {
    switch(type) {
        case lexic::type::REM:
            analyser_state = state::COMMENT;
            break;
        case lexic::type::INT:
        case lexic::type::PNT:
            analyser_state = state::NUMBER;
            break;
        default:
            analyser_state = state::NORMAL;
    }
}

token LexicalAnalyser::extract_token() {
    token lexeme;

    while (c.type == ascii_type::DELIMITER) {
        c = ac.get_next();
        if (c.character == EOF)
            return lexeme;
    }

    lexeme.set_position(c.pos);
    Lex(lexeme);

    return lexeme;
}

void LexicalAnalyser::Lex(token& lexeme) {
    if (c.type == ascii_type::LETTER) {
        Wrd(lexeme);
    }
    else if (c.type == ascii_type::DIGIT) {
        Int(lexeme);
    }
    else if (
        c.type == ascii_type::SPECIAL
        || c.type == ascii_type::LOWER
        || c.type == ascii_type::GREATER
        || c.type == ascii_type::EQUAL
    ) {
        Spe(lexeme);
    }
    else if (c.type == ascii_type::DQUOTE) {
        Strg(lexeme);
    }
}

void LexicalAnalyser::Wrd(token& lexeme) {
    //cout << "WRD | C = " << c.character << " | L = " << lexeme.value << endl;
    if (analyser_state == state::NUMBER && c.character == 'E' && ac.peek_next().type == ascii_type::DIGIT) {
        lexeme.add_char(c.character);
        c = ac.get_next();
    }
    else if (c.type == ascii_type::LETTER) {
        lexeme.add_char(c.character);
        c = ac.get_next();
        A(lexeme);
    }
}

void LexicalAnalyser::A(token& lexeme) {
    //cout << "A | C = " << c.character << " | L = " << lexeme.value << endl;
    if (c.type == ascii_type::LETTER) {
        lexeme.add_char(c.character);
        c = ac.get_next();
        A(lexeme);
    }
    else if (c.type == ascii_type::DIGIT) {
        lexeme.add_char(c.character);
        c = ac.get_next();
        A(lexeme);
    }
}

void LexicalAnalyser::Int(token& lexeme) {
    //cout << "INT | C = " << c.character << " | L = " << lexeme.value << endl;
    if (c.type == ascii_type::DIGIT) {
        lexeme.add_char(c.character);
        c = ac.get_next();
        B(lexeme);
    }
}

void LexicalAnalyser::B(token& lexeme) {
    //cout << "B | C = " << c.character << " | L = " << lexeme.value << endl;
    if (c.type == ascii_type::DIGIT) {
        lexeme.add_char(c.character);
        c = ac.get_next();
        B(lexeme);
    }
}

void LexicalAnalyser::Spe(token& lexeme) {
    //cout << "SPE | C = " << c.character << " | L = " << lexeme.value << endl;
    if (c.type == ascii_type::SPECIAL) {
        lexeme.add_char(c.character);
        c = ac.get_next();
    }
    if (c.type == ascii_type::LOWER) {
        lexeme.add_char(c.character);
        c = ac.get_next();
        C(lexeme);
    }
    if (c.type == ascii_type::GREATER) {
        lexeme.add_char(c.character);
        c = ac.get_next();
        D(lexeme);
    }
    if (c.type == ascii_type::EQUAL) {
        lexeme.add_char(c.character);
        c = ac.get_next();
    }
}

void LexicalAnalyser::C(token& lexeme) {
    //cout << "C | C = " << c.character << " | L = " << lexeme.value << endl;
    if (c.type == ascii_type::EQUAL)
        return;
    else if (c.type == ascii_type::GREATER)
        return;
}

void LexicalAnalyser::D(token& lexeme) {
    //cout << "D | C = " << c.character << " | L = " << lexeme.value << endl;
    if (c.type == ascii_type::LETTER)
        return;
}

void LexicalAnalyser::Strg(token& lexeme) {
    //cout << "STR | C = " << c.character << " | L = " << lexeme.value << endl;
    if (c.type == ascii_type::DQUOTE) {
        lexeme.add_char(c.character);
        c = ac.get_next();
        Chr(lexeme);
    }
}

void LexicalAnalyser::Chr(token& lexeme) {
    //cout << "CHR | C = " << c.character << " | L = " << lexeme.value << endl;
    if (c.type == ascii_type::LETTER) {
        lexeme.add_char(c.character);
        c = ac.get_next();
        Chr(lexeme);
    }
    else if (c.type == ascii_type::DIGIT) {
        lexeme.add_char(c.character);
        c = ac.get_next();
        Chr(lexeme);
    }
    else if (c.type == ascii_type::SPECIAL) {
        lexeme.add_char(c.character);
        c = ac.get_next();
        Chr(lexeme);
    }
    else if (c.type == ascii_type::GREATER) {
        lexeme.add_char(c.character);
        c = ac.get_next();
        Chr(lexeme);
    }
    else if (c.type == ascii_type::LOWER) {
        lexeme.add_char(c.character);
        c = ac.get_next();
        Chr(lexeme);
    }
    else if (c.type == ascii_type::EQUAL) {
        lexeme.add_char(c.character);
        c = ac.get_next();
        Chr(lexeme);
    }
    else if (c.type == ascii_type::DELIMITER) {
        lexeme.add_char(c.character);
        c = ac.get_next();
        Chr(lexeme);
    }
    else if (c.type == ascii_type::DQUOTE) {
        lexeme.add_char(c.character);
        c = ac.get_next();
    }
}

token LexicalAnalyser::read_comment() {
    token comment;

    ascii_character c = ac.get_next();
    comment.set_position(c.pos);
    while (c.character != 0xA) {
        comment.add_char(c.character);
        c = ac.get_next();

        if (file.eof())
            break;
    }
    comment.add_char(' ');
    return comment;
}

type LexicalAnalyser::categorize_token(string& value) {
    if (value == "LET")
        return type::LET;
    if (value == "FN")
        return type::FN;
    if (value == "DEF")
        return type::DEF;
    if (value == "READ")
        return type::READ;
    if (value == "DATA")
        return type::DATA;
    if (value == "PRINT")
        return type::PRINT;
    if (value == "GO")
        return type::GO;
    if (value == "TO")
        return type::TO;
    if (value == "GOTO")
        return type::GOTO;
    if (value == "IF")
        return type::IF;
    if (value == "THEN")
        return type::THEN;
    if (value == "FOR")
        return type::FOR;
    if (value == "STEP")
        return type::STEP;
    if (value == "NEXT")
        return type::NEXT;
    if (value == "DIM")
        return type::DIM;
    if (value == "GOSUB")
        return type::GOSUB;
    if (value == "RETURN")
        return type::RETURN;
    if (value == "REM")
        return type::REM;
    if (value == "END")
        return type::END;
    if (value == "+")
        return type::ADD;
    if (value == "-")
        return type::SUB;
    if (value == "*")
        return type::MUL;
    if (value == "/")
        return type::DIV;
    if (value == "^")
        return type::POW;
    if (value == "=")
        return type::EQL;
    if (value == "<>")
        return type::NEQ;
    if (value == "<")
        return type::LTN;
    if (value == ">")
        return type::GTN;
    if (value == "<=")
        return type::LEQ;
    if (value == ">=")
        return type::GEQ;
    if (value == ",")
        return type::COM;
    if (value == ".")
        return type::PNT;
    if (value == "(")
        return type::PRO;
    if (value == ")")
        return type::PRC;
    if (value == "SIN")
        return type::FNSIN;
    if (value == "COS")
        return type::FNCOS;
    if (value == "TAN")
        return type::FNTAN;
    if (value == "ATN")
        return type::FNATN;
    if (value == "EXP")
        return type::FNEXP;
    if (value == "ABS")
        return type::FNABS;
    if (value == "LOG")
        return type::FNLOG;
    if (value == "SQR")
        return type::FNSQR;
    if (value == "INT")
        return type::FNINT;
    if (value == "RND")
        return type::FNRND;
    if (value.c_str()[0] == '"')
        return type::STR;
    if (value == "E" && analyser_state == state::NUMBER)
        return type::EXD;
    if (value.c_str()[0] >= '0' && value.c_str()[0] <= '9')
        return type::INT;
    if (value != "")
        return type::IDN;
    return type::EoF;
}
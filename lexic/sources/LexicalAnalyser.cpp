#include <iostream>
#include <fstream>
#include <string>

#include "lexic.hpp"
#include "ASCIIClassifier.hpp"

#include "LexicalAnalyser.hpp"

using namespace std;
using namespace lexic;

LexicalAnalyser::LexicalAnalyser(ifstream& file):
    file(file), ac(file), analyser_state(state::NORMAL) {}

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

    while (ac.peek_next().type == ascii_type::DELIMITER) {
        ascii_character c = ac.get_next();
        if (c.character == EOF)
            return lexeme;
    }

    ascii_character c = ac.peek_next();
    lexeme.set_position(c.pos);

    if (analyser_state == state::NUMBER && c.character == 'E') {
        c = ac.get_next();
        lexeme.add_char(c.character);
        if (ac.peek_next().type == ascii_type::DIGIT || ac.peek_next().character == '+' || ac.peek_next().character == '-') {
            return lexeme;
        }
    }
 
    if (c.type == ascii_type::LETTER) {
        while (c.type == ascii_type::LETTER || c.type == ascii_type::DIGIT) {
            lexeme.add_char(ac.get_next().character);
            c = ac.peek_next();
        }
    }
    else if (c.type == ascii_type::DIGIT) {
        while (c.type == ascii_type::DIGIT) {
            lexeme.add_char(ac.get_next().character);
            c = ac.peek_next();
            if (c.type == ascii_type::LETTER) {
                if (c.character != 'E')
                    throw lexical_exception(lexeme.pos, "Identificador inválido");
                else
                    break;
            }
        }
    }
    else if (c.type == ascii_type::SPECIAL) {
        lexeme.add_char(ac.get_next().character);
        if (c.character == '<') {
            c = ac.peek_next();
            if (c.character == '=' || c.character == '>') {
                lexeme.add_char(ac.get_next().character);
            }
        } 
        else if (c.character == '>') {
            c = ac.peek_next();
            if (c.character == '=') {
                lexeme.add_char(ac.get_next().character);
            }
        }
        else if (c.character ==  '"') {
            do {
                c = ac.get_next();
                lexeme.add_char(c.character);
                if (c.character != '"' && ac.peek_next().character == EOF)
                    throw lexical_exception(lexeme.pos, "String não terminada. Esperando '\"'");
            } while (c.character != '"');
        }
    }
    else {
        throw lexical_exception(lexeme.pos, "Caractere não reconhecido");
    }

    return lexeme;
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
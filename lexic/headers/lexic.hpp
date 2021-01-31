#ifndef LEXIC_HPP
#define LEXIC_HPP

#include <string>
#include <exception>

namespace lexic {

enum class ascii_type {
    UNKNOWN,
    DIGIT,
    LETTER,
    SPECIAL,
    DELIMITER
};

enum class type {
    STR,        // Cadeia de carcteres
    INT,        // Sequência numérica
    IDN,        // Identificador
//Palavras reservadas:
    LET,        // Palavra reservada "LET"
    FN,         // Palavra reservada "FN"
    DEF,        // Palavra reservada "DEF"
    READ,       // Palavra reservada "READ"
    DATA,       // Palavra reservada "DATA"
    PRINT,      // Palavra reservada "PRINT"
    GO,         // Palavra reservada "GO"
    TO,         // Palavra reservada "TO"
    GOTO,       // Palavra reservada "GOTO"
    IF,         // Palavra reservada "IF"
    THEN,       // Palavra reservada "THEN"
    FOR,        // Palavra reservada "FOR"
    STEP,       // Palavra reservada "STEP"
    NEXT,       // Palavra reservada "NEXT"
    DIM,        // Palavra reservada "DIM"
    GOSUB,      // Palavra reservada "GOSUB"
    RETURN,     // Palavra reservada "RETURN"
    REM,        // Palavra reservada "REM"
    END,        // Palavra reservada "END"
// Símbolos:
    ADD,        // '+'  Operador de adição
    SUB,        // '-'  Operador de subtração
    MUL,        // '*'  Operador de multiplicação
    DIV,        // '/'  Operador de divisão
    POW,        // '^'  Operador de potenciação
    EQL,        // '='  Operador de igualdade
    NEQ,        // '<>' Operador de diferença
    LTN,        // '<'  Operador menor que
    GTN,        // '>'  Operador maior que
    LEQ,        // '<=' Operador menor que ou igual a
    GEQ,        // '>=' Operador maior que ou igual a
    COM,        // ','  Vírgula
    PNT,        // '.'  Ponto
    PRO,        // '('  Abre parênteses
    PRC,        // ')'  Fecha parênteses
    DQT,        // '"'  Aspas duplas
    EXD,        // 'E'  Expoente decimal para notação exponencial
// Funções prédefinidas
    FNSIN,      // Função Seno
    FNCOS,      // Função Cosseno
    FNTAN,      // Função Tangente
    FNATN,      // Função Arco-Tangente
    FNEXP,      // Função Exponencial
    FNABS,      // Função Valor Absoluto
    FNLOG,      // Função Logarítmo
    FNSQR,      // Função Raíz Quadrada
    FNINT,      // Função Valor Inteiro
    FNRND,      // Função Valor Aleatório
// Outros
    CMT,         // Comentários
    EoF         // Fim de Arquivo
};

enum class state {
    NORMAL,
    NUMBER,
    COMMENT
};

class position {
    public:
        int line, column;

        position() {};
        position(int line, int column):
            line(line), column(column) {}

        std::string position_str() {
            std::string s = "\033[37;1m(" + std::to_string(line) + "," + std::to_string(column) + ")\033[0m";
            return s;
        }
};

class ascii_character {
    public:
        char character;
        ascii_type type;
        position pos;
        
        ascii_character(char character, ascii_type type, position pos) :
            type(type), character(character), pos(pos) {}
};

class lexical_exception: public std::exception {
    public:
        lexical_exception(position& loc, const char* error_message)
            : loc(loc), error_message(error_message) {
            exception();
        }

        std::string message() {
            return loc.position_str() + ": " + error_message;
        }

    private:
        position& loc;
        const char* error_message;
};

class token {
    public:
        lexic::type type;
        std::string value;
        position pos;

        token():
            value("") {}

        void add_char(char c) {
            std::string new_value = value + c;
            value = new_value;
        }

        void set_position(position& p) {
            pos.line = p.line;
            pos.column = p.column;
        }
};

}; // namespace lexic

#endif // LEXIC_HPP
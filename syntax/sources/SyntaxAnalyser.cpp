#include <iostream>
#include <string>

#include "lexic.hpp"
#include "syntax.hpp"
#include "LexicalAnalyser.hpp"

#include "SyntaxAnalyser.hpp"

using namespace std;
using namespace syntax;

SyntaxAnalyser::SyntaxAnalyser(ifstream& file):
    file(file), lex(file) {}


bool SyntaxAnalyser::file_end() {
    if (token_consumed) {
        tk = lex.get_next();
        token_consumed = false;
    }
    return tk.type == lexic::type::EoF;
}

BStatement* SyntaxAnalyser::get_next() {
    int index;

    if (file_end())
        return nullptr;

    consume(lexic::type::INT, method::REQUIRED);
    index = stoi(tk.value);

    tk = lex.get_next();

    switch (tk.type) {
        case lexic::type::LET:
            return parse_assign(index, tk.pos);
        case lexic::type::READ:
            return parse_read(index, tk.pos);
        case lexic::type::DATA:
            return parse_data(index, tk.pos);
        case lexic::type::PRINT:
            return parse_print(index, tk.pos);
        case lexic::type::GO:
        case lexic::type::GOTO:
            return parse_goto(index, tk.pos);
        case lexic::type::IF:
            return parse_if(index, tk.pos);
        case lexic::type::FOR:
            return parse_for(index, tk.pos);
        case lexic::type::NEXT:
            return parse_next(index, tk.pos);
        case lexic::type::DIM:
            return parse_dim(index, tk.pos);
        case lexic::type::DEF:
            return parse_def(index, tk.pos);
        case lexic::type::GOSUB:
            return parse_gosub(index, tk.pos);
        case lexic::type::RETURN:
            return parse_return(index, tk.pos);
        case lexic::type::REM:
            return parse_rem(index, tk.pos);
        case lexic::type::END:
            return parse_end(index, tk.pos);
        case lexic::type::EoF:
            return nullptr;
        default:
            throw syntax_exception(tk.pos, string("Token inesperado: ") + tk.value);
    }
}


Assign* SyntaxAnalyser::parse_assign(int index, lexic::position pos) {
    Var* variable;
    Exp* expression;

    consume(lexic::type::IDN, method::REQUIRED);
    variable = new Var(Elem::VAR, tk.pos, false, tk.value);

    consume(lexic::type::EQL, method::REQUIRED);

    expression = parse_exp();

    return new Assign(index, pos, variable, expression);
}

Read* SyntaxAnalyser::parse_read(int index, lexic::position pos) {
    vector<Var*> variables;

    variables.push_back(parse_var());

    while(consume(lexic::type::COM, method::OPTIONAL)) {
        variables.push_back(parse_var());
    }

    return new Read(index, pos, variables);
}

Data* SyntaxAnalyser::parse_data(int index, lexic::position pos) {
    vector<Num*> values;

    Num* n = parse_snum();
    values.push_back(n);

    while (consume(lexic::type::COM, method::OPTIONAL)) {
        Num* n = parse_snum();
        values.push_back(n);
    }

    return new Data(index, pos, values);
}

Pitem* SyntaxAnalyser::parse_pitem() {
    Exp* exp;
    string str;

    if (consume(lexic::type::STR, method::OPTIONAL)) {
        str = tk.value;
        return new Pitem(str);
    }
    else if (consume(lexic::type::INT, method::LOOKAHEAD)
        || consume(lexic::type::IDN, method::LOOKAHEAD)
        || consume(lexic::type::FN, method::LOOKAHEAD)
        || consume(lexic::type::PRO, method::LOOKAHEAD)
        ) {
        exp = parse_exp();
        return new Pitem(exp);
    }
}

Print* SyntaxAnalyser::parse_print(int index, lexic::position pos) {
    vector<Pitem*> pitems;

    pitems.push_back(parse_pitem());
    while (consume(lexic::type::COM, method::OPTIONAL)) {
        pitems.push_back(parse_pitem());
    }

    return new Print(index, pos, pitems);
}

Goto* SyntaxAnalyser::parse_goto(int index, lexic::position pos) {
    int destination;

    if (consume(lexic::type::TO, method::OPTIONAL)) {
        consume(lexic::type::INT, method::REQUIRED);
        destination = stoi(tk.value);
    }
    else {
        consume(lexic::type::INT, method::REQUIRED);
        destination = stoi(tk.value);
    }

    return new Goto(index, pos, destination);
}

If* SyntaxAnalyser::parse_if(int index, lexic::position pos) {
    Exp *left, *right;
    If::cmp op;
    int destination;

    left = parse_exp();

    if (consume(lexic::type::EQL, method::OPTIONAL)) {
        op = If::EQL;
    }
    else if (consume(lexic::type::NEQ, method::OPTIONAL)) {
        op = If::NEQ;
    }
    else if (consume(lexic::type::LTN, method::OPTIONAL)) {
        op = If::LTN;
    }
    else if (consume(lexic::type::GTN, method::OPTIONAL)) {
        op = If::GTN;
    }
    else if (consume(lexic::type::LEQ, method::OPTIONAL)) {
        op = If::LEQ;
    }
    else if (consume(lexic::type::GEQ, method::OPTIONAL)) {
        op = If::GEQ;
    }

    right = parse_exp();

    consume(lexic::type::THEN, method::REQUIRED);

    consume(lexic::type::INT, method::REQUIRED);
    destination = stoi(tk.value);

    return new If(index, pos, left, op, right, destination);
}

For* SyntaxAnalyser::parse_for(int index, lexic::position pos) {
    Var* iterator;
    Exp *init, *step, *stop;

    consume(lexic::type::IDN, method::REQUIRED);
    iterator = new Var(Elem::VAR, tk.pos, false, tk.value);

    consume(lexic::type::EQL, method::REQUIRED);
    init = parse_exp();

    consume(lexic::type::TO, method::REQUIRED);
    stop = parse_exp();

    if (consume(lexic::type::STEP, method::OPTIONAL)) {
        step = parse_exp();
    }
    else {
        vector<Operator*> operators;
        vector<Eb*> operands;
        operands.push_back(new Num(Elem::NUM, 1, 0, false, 0));

        step = new Exp(Elem::EXP, false, operands, operators);
    }

    return new For(index, pos, iterator, init, stop, step);
}

Next* SyntaxAnalyser::parse_next(int index, lexic::position pos) {
    Var* iterator;

    consume(lexic::type::IDN, method::REQUIRED);
    iterator = new Var(Elem::VAR, tk.pos, false, tk.value);

    return new Next(index, pos, iterator);
}

Array* SyntaxAnalyser::parse_array() {
    string identifier;
    vector<int> dimensions;
    lexic::position pos;

    consume(lexic::type::IDN, method::REQUIRED);
    identifier = tk.value;
    pos = tk.pos;

    consume(lexic::type::PRO, method::REQUIRED);

    consume(lexic::type::INT, method::REQUIRED);
    dimensions.push_back(stoi(tk.value));

    while (consume(lexic::type::COM, method::OPTIONAL)) {
        consume(lexic::type::INT, method::REQUIRED);
        dimensions.push_back(stoi(tk.value));
    }
    consume(lexic::type::PRC, method::REQUIRED);

    return new Array(Elem::VAR, pos, identifier, dimensions);
}

Dim* SyntaxAnalyser::parse_dim(int index, lexic::position pos) {
    vector<Array*> arrays;

    arrays.push_back(parse_array());

    while (consume(lexic::type::COM, method::OPTIONAL)) {
        arrays.push_back(parse_array());
    }

    return new Dim(index, pos, arrays);
}

Def* SyntaxAnalyser::parse_def(int index, lexic::position pos) {
    string identifier;
    vector<Var*> parameters;

    consume(lexic::type::FN, method::REQUIRED);

    consume(lexic::type::IDN, method::REQUIRED);
    identifier = tk.value;

    consume(lexic::type::PRO, method::REQUIRED);

    if (consume(lexic::type::IDN, method::OPTIONAL)) {
        parameters.push_back(new Var(Elem::VAR, tk.pos, false, tk.value));

        while (consume(lexic::type::COM, method::OPTIONAL)) {
            consume(lexic::type::IDN, method::REQUIRED);
            parameters.push_back(new Var(Elem::VAR, tk.pos, false, tk.value));
        }
    }

    consume(lexic::type::PRC, method::REQUIRED);

    consume(lexic::type::EQL, method::REQUIRED);
    Exp* exp = parse_exp();

    return new Def(index, pos, identifier, parameters, exp);
}

Gosub* SyntaxAnalyser::parse_gosub(int index, lexic::position pos) {
    int destination;

    consume(lexic::type::INT, method::REQUIRED);
    destination = stoi(tk.value);

    return new Gosub(index, pos, destination);
}

Return* SyntaxAnalyser::parse_return(int index, lexic::position pos) {
    return new Return(index, pos);
}

Rem* SyntaxAnalyser::parse_rem(int index, lexic::position pos) {
    consume(lexic::type::CMT, method::OPTIONAL);

    return new Rem(index, pos);
}

End* SyntaxAnalyser::parse_end(int index, lexic::position pos) {
    return new End(index, pos);
}

Exp* SyntaxAnalyser::parse_exp() {
    bool negative = false;
    std::vector<Eb*> operands;
    std::vector<Operator*> operators;

    if (consume(lexic::type::ADD, method::OPTIONAL))
        negative = false;
    else if (consume(lexic::type::SUB, method::OPTIONAL))
        negative = true;

    operands.push_back(parse_eb());

    while (true) {
        Operator* op = parse_operator();

        if(op == nullptr)
            break;

        operators.push_back(op);
        operands.push_back(parse_eb());
    }

    return new Exp(Elem::EXP, negative, operands, operators);
}

Operator* SyntaxAnalyser::parse_operator() {
    if (consume(lexic::type::ADD, method::OPTIONAL)) {
        return new Operator(Elem::ADD, Operator::ADD, tk.value);
    }
    else if (consume(lexic::type::SUB, method::OPTIONAL)) {
        return new Operator(Elem::SUB, Operator::SUB, tk.value);
    }
    else if (consume(lexic::type::MUL, method::OPTIONAL)) {
        return new Operator(Elem::MUL, Operator::MUL, tk.value);
    }
    else if (consume(lexic::type::DIV, method::OPTIONAL)) {
        return new Operator(Elem::DIV, Operator::DIV, tk.value);
    }
    else if (consume(lexic::type::POW, method::OPTIONAL)) {
        return new Operator(Elem::POW, Operator::POW, tk.value);
    }
    else {
        return nullptr;
    }
}

Eb* SyntaxAnalyser::parse_eb() {
    if (consume(lexic::type::INT, method::LOOKAHEAD)
        || consume(lexic::type::PNT, method::LOOKAHEAD)
        ) {
        return parse_num();
    }
    else if (consume(lexic::type::IDN, method::LOOKAHEAD)) {
        return parse_var();
    }
    else if (consume(lexic::type::PRO, method::OPTIONAL)) {
        Exp* exp = parse_exp();
        consume(lexic::type::PRC, method::REQUIRED);
        return exp;
    }
    else if (consume(lexic::type::FN, method::LOOKAHEAD)
            || consume(lexic::type::FNSIN, method::LOOKAHEAD)
            || consume(lexic::type::FNCOS, method::LOOKAHEAD)
            || consume(lexic::type::FNTAN, method::LOOKAHEAD)
            || consume(lexic::type::FNATN, method::LOOKAHEAD)
            || consume(lexic::type::FNEXP, method::LOOKAHEAD)
            || consume(lexic::type::FNABS, method::LOOKAHEAD)
            || consume(lexic::type::FNLOG, method::LOOKAHEAD)
            || consume(lexic::type::FNSQR, method::LOOKAHEAD)
            || consume(lexic::type::FNINT, method::LOOKAHEAD)
            || consume(lexic::type::FNRND, method::LOOKAHEAD)
        ) {
        return parse_call();
    }
    else {
        throw syntax_exception(tk.pos, "Encontrado '" + tk.value + "' em posição inesperada");
    }
}

Num* SyntaxAnalyser::parse_snum() {
    bool negative = false;

    if (consume(lexic::type::ADD, method::OPTIONAL)) {
        negative = false;
    }
    else if (consume(lexic::type::SUB, method::OPTIONAL)) {
        negative = true;
    }

    Num* n = parse_num();

    return new Num(negative, n);
}

Num* SyntaxAnalyser::parse_num() {
    int integer = 0, exponent = 0;
    double frac = 0.0;
    bool neg_exp = false;

    if (consume(lexic::type::PNT, method::OPTIONAL)) {
        consume(lexic::type::INT, method::REQUIRED);
        frac = stod("0." + tk.value);
    }
    else if (consume(lexic::type::INT, method::OPTIONAL)) {
        integer = stoi(tk.value);

        if (consume(lexic::type::PNT, method::OPTIONAL)) {
            consume(lexic::type::INT, method::REQUIRED);
            frac = stod("0." + tk.value);
        }
    }
    else {
        throw syntax_exception(tk.pos, "Esperado encontrar um número");
    }

    if (consume(lexic::type::EXD, method::OPTIONAL)) {
        if (consume(lexic::type::ADD, method::OPTIONAL))
            neg_exp = false;

        else if (consume(lexic::type::SUB, method::OPTIONAL))
            neg_exp = true;

        consume(lexic::type::INT, method::REQUIRED);
        exponent = stoi(tk.value);
    }

    return new Num(Elem::NUM, integer, frac, neg_exp, exponent);
}

Var* SyntaxAnalyser::parse_var() {
    string identifier;

    if (consume(lexic::type::IDN, method::REQUIRED))
        identifier = tk.value;

    if (consume(lexic::type::PRO, method::OPTIONAL)) {
        int dimension = 1;
        vector<Exp*> access_exps;

        access_exps.push_back(parse_exp());
        while (consume(lexic::type::COM, method::OPTIONAL)) {
            access_exps.push_back(parse_exp());
            dimension++;
        }
        consume(lexic::type::PRC, method::OPTIONAL);

        return new ArrayAccess(Elem::VAR, tk.pos, identifier, dimension, access_exps);
    }

    return new Var(Elem::VAR, tk.pos, false, identifier);
}

Call* SyntaxAnalyser::parse_call() {
    vector<Exp*> args;
    string identifier;

    if (consume(lexic::type::FN, method::OPTIONAL)) {
        consume(lexic::type::IDN, method::REQUIRED);
        identifier = tk.value;
    }
    else if (consume(lexic::type::FNSIN, method::OPTIONAL)) {
        identifier = "SIN";
    }
    else if (consume(lexic::type::FNCOS, method::OPTIONAL)) {
        identifier = "COS";
    }
    else if (consume(lexic::type::FNTAN, method::OPTIONAL)) {
        identifier = "TAN";
    }
    else if (consume(lexic::type::FNATN, method::OPTIONAL)) {
        identifier = "ATN";
    }
    else if (consume(lexic::type::FNEXP, method::OPTIONAL)) {
        identifier = "EXP";
    }
    else if (consume(lexic::type::FNABS, method::OPTIONAL)) {
        identifier = "ABS";
    }
    else if (consume(lexic::type::FNLOG, method::OPTIONAL)) {
        identifier = "LOG";
    }
    else if (consume(lexic::type::FNSQR, method::OPTIONAL)) {
        identifier = "SQR";
    }
    else if (consume(lexic::type::FNINT, method::OPTIONAL)) {
        identifier = "INT";
    }
    else if (consume(lexic::type::FNRND, method::OPTIONAL)) {
        identifier = "RND";
    }

    consume(lexic::type::PRO, method::REQUIRED);

    // Função sem argumentos
    if (consume(lexic::type::PRC, method::OPTIONAL))
        return new Call(Elem::FUN, tk.pos, identifier, args);

    args.push_back(parse_exp());
    while (consume(lexic::type::COM, method::OPTIONAL)) {
        args.push_back(parse_exp());
    }

    consume(lexic::type::PRC, method::REQUIRED);

    return new Call(Elem::FUN, tk.pos, identifier, args);
}

bool SyntaxAnalyser::consume(lexic::type type, method m) {
    lexic::position pos = tk.pos;

    if (token_consumed)
        tk = lex.get_next();

    bool match = (tk.type == type) ? true : false;

    token_consumed = (m == method::LOOKAHEAD) ? false : match;

    if (m == method::REQUIRED && !token_consumed)
        throw syntax_exception(tk.pos, string("Token inesperado: ") + tk.value);

    return match;
}

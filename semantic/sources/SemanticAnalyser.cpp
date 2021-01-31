#include <iostream>
#include <string.h>
#include <utility>
#include <vector>
#include <set>

#include "syntax.hpp"
#include "semantic.hpp"

#include "SemanticAnalyser.hpp"

using namespace std;
using namespace syntax;
using namespace semantic;
using namespace generation;



auto cmp = [](syntax::BStatement* a, syntax::BStatement* b) {
    if (a->get_index() == b->get_index()) {
        throw semantic_exception(a->get_position(), "Índice de linha já existente");
    }
    return a->get_index() < b->get_index();
};
set<syntax::BStatement*, decltype(cmp)> statements(cmp);

SemanticAnalyser::SemanticAnalyser(ifstream& input, SymbolTable& symb_table, CodeGenerator& gen):
    stx(input), symb_table(symb_table), gen(gen)
{}

SemanticAnalyser::~SemanticAnalyser() {
    while (!statements.empty()) {
        set<BStatement*>::iterator it = statements.begin();
        statements.erase(it);
        delete_statement(*it);
    }
}

int find_next_index(BStatement* current = nullptr) {
    int read = false;
    int data = false;
    for (auto statement : statements) {
        if (dynamic_cast<Print*>(statement)
            || dynamic_cast<Dim*>(statement)
            || dynamic_cast<Def*>(statement)
            || dynamic_cast<Rem*>(statement))
            continue;

        if (dynamic_cast<Read*>(statement)) {
            read = true;
            if (!data)
                continue;
        }

        if (dynamic_cast<Data*>(statement)) {
            data = true;
            if (!read)
                continue;
        }

        if (!current) {
            return statement->get_index();
        }
        else if (statement->get_index() > current->get_index()) {
            return statement->get_index();
        }
    }
}

void SemanticAnalyser::run() {
    BStatement* sx;

    while (true) {
        sx = stx.get_next();

        if (sx == nullptr)
            break;

        if (dynamic_cast<Rem*>(sx)) {
            delete sx;
            continue;
        }

        statements.insert(sx);
    }

    if (!dynamic_cast<End*>(*statements.rbegin()))
        throw semantic_exception((*statements.rbegin())->get_position(), "Programa não termina com comando END");

    gen.generate_header(find_next_index());

    bool ended = false;
    for (auto command : statements) {
        if (ended)
            throw semantic_exception(command->get_position(), "Programa continua após o comando END");

        if (command == nullptr)
            cout << "sintaxema nulo?!" << endl;

        if (Assign* assign = dynamic_cast<Assign*>(command)) {
            process_assign(assign);
        }
        else if (Read* read = dynamic_cast<Read*>(command)) {
            process_read(read);
        }
        else if (Data* data = dynamic_cast<Data*>(command)) {
            process_data(data);
        }
        else if (Print* print = dynamic_cast<Print*>(command)) {
            process_print(print);
        }
        else if (Goto* go = dynamic_cast<Goto*>(command)) {
            process_goto(go);
        }
        else if (If* ift = dynamic_cast<If*>(command)) {
            process_if(ift);
        }
        else if (For* loop = dynamic_cast<For*>(command)) {
            process_for(loop);
        }
        else if (Next* next = dynamic_cast<Next*>(command)) {
            process_next(next);
        }
        else if (Dim* dim = dynamic_cast<Dim*>(command)) {
            process_dim(dim);
        }
        else if (Def* def = dynamic_cast<Def*>(command)) {
            process_def(def);
        }
        else if (Gosub* gosub = dynamic_cast<Gosub*>(command)) {
            process_gosub(gosub);
        }
        else if (Return* ret = dynamic_cast<Return*>(command)) {
            process_return(ret);
        }
        else if (End* end = dynamic_cast<End*>(command)) {
            ended = true;
            process_end(end);
        }
        else {
            cout << "É outra coisa" << endl;
        }
    }

    gen.generate_variables();

    symb_table.print_variables();
}

void SemanticAnalyser::process_assign(syntax::Assign* assign) {
    Var* decl = symb_table.pointer_to_variable(assign->get_variable());
    if (decl != nullptr && decl->is_array())
        throw semantic_exception(assign->get_position(), "Variáveis indexadas não podem ser atribuídas em LET");

    process_variable(assign->get_variable());
    vector<Elem*> exp = process_expression(assign->get_expression());

    int next_index = find_next_index(assign);
    gen.generate(assign, exp, next_index);
}

void SemanticAnalyser::process_read(syntax::Read* read) {
    for (auto var: read->get_variables()) {
        if (var->is_array()) {
            if (symb_table.select_variable(var) == 0)
                throw semantic_exception(var->get_position(), "Atribuição de variável indexada não declarada '" + var->get_identifier() + "' não é permitida");

            Array* decl = dynamic_cast<Array*>(symb_table.pointer_to_variable(var));
            ArrayAccess* access = dynamic_cast<ArrayAccess*>(var);
            access->set_array(decl);

            vector<Elem*> processed_array_exps;
            processed_array_exps = process_array_access_exp(access);

            access->set_processed_access_exps(processed_array_exps);
        }

        read_variables.push(var);
        process_variable(var);
    }

    vector<pair<Var*,Num*>> read_data;
    while (!read_variables.empty() && !data_values.empty()) {
        Var* var = read_variables.front();
        Num* val = data_values.front();

        //cout << "\tVAR[" << var->get_index() << "] " << var->get_identifier() << " = " << val->get_value() << endl;

        read_data.push_back(make_pair(var, val));

        read_variables.pop();
        data_values.pop();
    }

    int next_index = find_next_index(read);
    if (!read_data.empty())
        gen.generate(read, read_data, next_index);
}

void SemanticAnalyser::process_data(syntax::Data* data) {
    for (auto val: data->get_values()) {
        data_values.push(val);
    }

    vector<pair<Var*,Num*>> read_data;

    while (!read_variables.empty() && !data_values.empty()) {
        Var* var = read_variables.front();
        Num* val = data_values.front();

        //cout << "\tVAR[" << var->get_index() << "] " << var->get_identifier() << " = " << val->get_value() << endl;

        read_data.push_back(make_pair(var, val));

        read_variables.pop();
        data_values.pop();
    }

    int next_index = find_next_index(data);
    if (!read_data.empty())
        gen.generate(data, read_data, next_index);
}

void SemanticAnalyser::process_print(syntax::Print* print) {
    cout << "Geração para PRINT não implementada" << endl;
}

void SemanticAnalyser::process_goto(Goto* go) {
    for (set<BStatement*>::iterator it = statements.begin(); it != statements.end(); ++it) {
        if (go->get_destination() == (*it)->get_index()) {
            int destination = find_next_index(*(--it));
            gen.generate(go, destination);
            return;
        }
    }

    throw semantic_exception(go->get_position(), string("Comando GOTO com linha de destino inexistente"));
}

void SemanticAnalyser::process_if(syntax::If* ift) {
    vector<Elem*> left = process_expression(ift->get_left());
    vector<Elem*> right = process_expression(ift->get_right());

    int next_index = find_next_index(ift);

    for (set<BStatement*>::iterator it = statements.begin(); it != statements.end(); ++it) {
        if (ift->get_destination() == (*it)->get_index()) {
            int destination = find_next_index(*(--it));
            gen.generate(ift, left, right, destination, next_index);
            return;
        }
    }

    throw semantic_exception(ift->get_position(), string("Comando IF com linha de destino inexistente"));
}

void SemanticAnalyser::process_for(For* loop) {
    if (symb_table.select_variable(loop->get_iterator()))
        throw semantic_exception(loop->get_position(), "Variável de iteração " + loop->get_iterator()->get_identifier() + " já declarada");

    process_variable(loop->get_iterator());

    for_stack.push_back(loop);
}

void SemanticAnalyser::process_next(Next* next) {
    if (for_stack.empty())
        throw semantic_exception(next->get_position(), "NEXT sem FOR correspondente");

    process_variable(next->get_iterator());

    For* loop = for_stack.back();

    if (symb_table.select_variable(next->get_iterator())
        != symb_table.select_variable(loop->get_iterator()))
        throw semantic_exception(next->get_position(), "NEXT para laço não imediatamente anterior");

    // Geração do FOR correspondente
    vector<Elem*> init = process_expression(loop->get_init());
    vector<Elem*> stop = process_expression(loop->get_stop());
    vector<Elem*> step = process_expression(loop->get_step());
    int index_inside_loop = find_next_index(loop);
    int index_outside_loop = find_next_index(next);
    gen.generate(loop, init, stop, step, index_inside_loop, index_outside_loop);
    
    // Geração do NEXT
    next->set_loop(loop);
    gen.generate(next);

    for_stack.pop_back();
}

void SemanticAnalyser::process_dim(Dim* dim) {
    for (auto array : dim->get_arrays()) {

        int ret = symb_table.select_variable(array);
        if (ret != 0)
            throw semantic_exception(array->get_position(), "Variável " + array->get_identifier() + " já foi declarada");

        process_variable(array);
    }
}

void identify_def_parameters(Def* def, Exp* exp) {
    for (auto operand : exp->get_operands()) {
        if (Var* v = dynamic_cast<Var*>(operand)) {
            for (auto parameter : def->get_parameters()) {
                if (v->get_identifier() == parameter->get_identifier())
                    v->make_parameter(def->get_identifier());
            }
        }
        else if (Exp* e = dynamic_cast<Exp*>(operand)) {
            identify_def_parameters(def, e);
        }
    }
}

void SemanticAnalyser::process_def(Def* def) {
    if (!symb_table.insert_function(def))
        throw semantic_exception(def->get_position(), "Declaração dupla para função " + def->get_identifier());

    identify_def_parameters(def, def->get_exp());

    for (auto parameter : def->get_parameters()) {
        parameter->make_parameter(def->get_identifier());
        process_variable(parameter);
    }

    vector<Elem*> exp = process_expression(def->get_exp());

    gen.generate(def, exp);
}

void SemanticAnalyser::process_gosub(syntax::Gosub* gosub) {
    for (set<BStatement*>::iterator it = statements.begin(); it != statements.end(); ++it) {
        if (gosub->get_destination() == (*it)->get_index()) {
            int destination = find_next_index(*(--it));
            gen.generate(gosub, destination);
            return;
        }
    }

    throw semantic_exception(gosub->get_position(), string("Comando GOSUB com subrotina de destino inexistente"));
}

void SemanticAnalyser::process_return(syntax::Return* ret) {
    gen.generate(ret);
}

void SemanticAnalyser::process_end(syntax::End* end) {
    if (!read_variables.empty())
        throw semantic_exception(end->get_position(), "Fim de programa atingido com " + to_string(read_variables.size()) + " variável(is) aguardando em READ");
    else if (!for_stack.empty())
        throw semantic_exception(end->get_position(), "Fim de programa atingido com laço FOR não terminado");

    gen.generate(end);
}


void SemanticAnalyser::process_variable(Var* v) {
    int index;
    if (Array* a = dynamic_cast<Array*>(v)) {
        index = symb_table.insert_array(a);
    }
    else {
        index = symb_table.insert_variable(v);
    }
    v->set_index(index);
}

vector<Elem*> SemanticAnalyser::process_expression(Exp* e) {
    vector<syntax::Elem*> exp;

    gen_exp_vector(e, exp);

    return convert_to_postfix(exp);
}

string read_elem_type(syntax::Elem* e) {
    switch (e->get_elem_type()) {
        case syntax::Elem::NUM: return to_string(dynamic_cast<syntax::Num*>(e)->get_value());
        case syntax::Elem::VAR: return dynamic_cast<syntax::Var*>(e)->get_identifier();
        case syntax::Elem::FUN: return dynamic_cast<syntax::Call*>(e)->get_identifier();
        case syntax::Elem::ADD: return "+";
        case syntax::Elem::SUB: return "-";
        case syntax::Elem::MUL: return "*";
        case syntax::Elem::DIV: return "/";
        case syntax::Elem::POW: return "^";
        case syntax::Elem::PRO: return "(";
        case syntax::Elem::PRC: return ")";
        case syntax::Elem::COM: return ",";
    }
}

void print_exp(const vector<syntax::Elem*>& exp) {
    cout << "[ ";
    for (auto elem : exp) {
        cout << read_elem_type(elem) << " ";
    }
    cout << "]" << endl;
}

void SemanticAnalyser::gen_exp_vector(syntax::Exp* e, vector<syntax::Elem*>& exp) {

    if (e->is_negative()) {
        gen_negative_exp_vector(e, exp);
        return;
    }

    vector<Eb*> operands = e->get_operands();
    vector<Operator*> operators = e->get_operators();

    //TODO: Retornar erro se não houver operandos
    Eb* operand = operands.front();
    gen_exp_vector_operand(operand, exp);

    int size = operators.size();
    for (int i = 0; i < size; i++) {
        exp.push_back(operators.at(i));

        Eb* operand = operands.at(i + 1);
        gen_exp_vector_operand(operand, exp);
    }
}

void SemanticAnalyser::gen_exp_vector_operand(syntax::Eb* operand, vector<syntax::Elem*>& exp) {
    if (operand->get_eb_type() == Eb::EXP) {
        exp.push_back(new Elem(Elem::PRO));
        gen_exp_vector(dynamic_cast<Exp*>(operand), exp);
        exp.push_back(new Elem(Elem::PRC));
    }
    else if (operand->get_eb_type() == Eb::CALL) {
        Call* c = dynamic_cast<Call*>(operand);
        if (!symb_table.select_function(c))
            throw semantic_exception(c->get_position(), "Função '" + c->get_identifier() + "' não declarada");

        Def* decl = symb_table.select_function(c);
        if (decl->get_parameters().size() != c->get_args().size())
            throw semantic_exception(c->get_position(), "Lista de parâmetros incompatível com a declaração de '" + decl->get_identifier() + "'");

        exp.push_back(operand);
        exp.push_back(new Elem(Elem::PRO));

        vector<Exp*> args = dynamic_cast<Call*>(operand)->get_args();

        if (!args.empty()) {
            gen_exp_vector(args.at(0), exp);
            for (int i = 1; i < args.size(); i++) {
                exp.push_back(new Elem(Elem::COM));
                gen_exp_vector(args.at(i), exp);
            }
        }

        exp.push_back(new Elem(Elem::PRC));
    }
    else if (operand->get_eb_type() == Eb::VAR) {
        Var* v = dynamic_cast<Var*>(operand);
        if (symb_table.select_variable(v) == 0)
                throw semantic_exception(v->get_position(), string("Variável '" + v->get_identifier() + "' não declarada"));

        if (v->is_array()) {
            exp.push_back(operand);

            Array* decl = dynamic_cast<Array*>(symb_table.pointer_to_variable(v));
            ArrayAccess* access = dynamic_cast<ArrayAccess*>(v);

            if (decl->get_dimensions().size() != access->get_dimension())
                throw semantic_exception(v->get_position(), "Lista de acesso a variável indexada incompatível com a declaração de '" + v->get_identifier() + "'");

            access->set_array(decl);
            int dimensions = access->get_access_exps().size();

            exp.push_back(new Elem(Elem::PRO));

            for (int i = 0; i < dimensions; i++) {
                if (i != 0)
                    exp.push_back(new Elem(Elem::ADD));

                exp.push_back(new Elem(Elem::PRO));
                gen_exp_vector(access->get_access_exps().at(i), exp);
                exp.push_back(new Elem(Elem::PRC));

                for ( int j = i + 1; j < dimensions; j++) {
                    exp.push_back(new Elem(Elem::MUL));
                    exp.push_back(new Num(Elem::NUM, decl->get_dimensions().at(j), 0, false, 0));
                }
            }

            exp.push_back(new Elem(Elem::PRC));
        }
        else {
            exp.push_back(operand);
        }
    }
    else {
        exp.push_back(operand);
    }
}

void SemanticAnalyser::gen_negative_exp_vector(syntax::Exp* e, std::vector<syntax::Elem*>& exp) {
    vector<Operator*> operators;
    Operator* sub = new Operator(Elem::SUB, Operator::SUB, "-");
    operators.push_back(sub);

    vector<Eb*> operands;
    Eb* zero = new Num(Elem::NUM, 0, 0, false, 0);
    operands.push_back(zero);
    e->make_positive();
    operands.push_back(e);

    Exp* neg_exp = new Exp(Elem::EXP, false, operands, operators);

    gen_exp_vector(neg_exp, exp);
}

vector<Elem*> SemanticAnalyser::process_array_access_exp(ArrayAccess* access) {
    vector<Elem*> processed_access_exps;

    int dimensions = access->get_access_exps().size();
    Array* decl = access->get_array();

    for (int i = 0; i < dimensions; i++) {
        vector<Elem*> ae = process_expression(access->get_access_exps().at(i));
        for (auto elem : ae) {
            processed_access_exps.push_back(elem);
        }

        for ( int j = i + 1; j < dimensions; j++) {
            processed_access_exps.push_back(new Num(Elem::NUM, decl->get_dimensions().at(j), 0, false, 0));
            processed_access_exps.push_back(new Elem(Elem::MUL));
        }

        if (i != 0)
            processed_access_exps.push_back(new Elem(Elem::ADD));

    }

    return processed_access_exps;
}

int precedence(syntax::Elem* e) {
    switch (e->get_elem_type()) {
        case syntax::Elem::ADD: 
            return 2;
        case syntax::Elem::SUB: 
            return 2;
        case syntax::Elem::MUL: 
            return 3;
        case syntax::Elem::DIV: 
            return 3;
        case syntax::Elem::POW: 
            return 4;
        default:
            return 0;
    }
}

enum assoc {
    LEFT,
    RIGHT
};

int associativity(syntax::Elem* e) {
    switch (e->get_elem_type()) {
        case syntax::Elem::ADD: 
            return LEFT;
        case syntax::Elem::SUB: 
            return LEFT;
        case syntax::Elem::MUL: 
            return LEFT;
        case syntax::Elem::DIV: 
            return LEFT;
        case syntax::Elem::POW: 
            return RIGHT;
        default:
            return LEFT;
    }
}


vector<syntax::Elem*> SemanticAnalyser::convert_to_postfix(vector<syntax::Elem*>& infix) {
    // Based on shunting yard algorithm by Edsger Dijkstra

    vector<syntax::Elem*> postfix, stack;

    //cout << "infix: ";
    //print_exp(infix);
    //cout << "stack: ";
    //print_exp(stack);
    //cout << "postfix: ";
    //print_exp(postfix);
    //cout << endl;

    while (!infix.empty()) {
        syntax::Elem* e = infix.front();
        infix.erase(infix.begin());

        if (e->get_elem_type() == syntax::Elem::NUM) {
            postfix.push_back(e);
        }
        else if (e->get_elem_type() == syntax::Elem::VAR) {
            Var* v = dynamic_cast<Var*>(e);

            if (v->is_array())
                stack.push_back(v);
            else
                postfix.push_back(e);
        }
        else if (e->get_elem_type() == Elem::FUN) {
            stack.push_back(e);
        }
        else if (e->is_operator()) {
            while ( !stack.empty()
                && (stack.back()->is_operator())
                && ((precedence(stack.back()) > precedence(e)) || (precedence(stack.back()) == precedence(e) && associativity(e) == LEFT))
                && (stack.back()->get_elem_type() != syntax::Elem::PRO)
                ) {
                postfix.push_back(stack.back());
                stack.pop_back();
            }
            stack.push_back(e);
        }
        else if (e->get_elem_type() == syntax::Elem::PRO) {
            stack.push_back(e);
        }
        else if (e->get_elem_type() == syntax::Elem::COM) {
            while (stack.back()->get_elem_type() != syntax::Elem::PRO) {
                postfix.push_back(stack.back());
                stack.pop_back();
            }
        }
        else if (e->get_elem_type() == syntax::Elem::PRC) {
            while (stack.back()->get_elem_type() != syntax::Elem::PRO) {
                postfix.push_back(stack.back());
                stack.pop_back();
            }
            if (stack.back()->get_elem_type() == syntax::Elem::PRO) {
                stack.pop_back();
            }
            if (!stack.empty()
                && (stack.back()->get_elem_type() == syntax::Elem::FUN
                    || stack.back()->get_elem_type() == syntax::Elem::VAR) // Variáveis indexadas
                ) {
                postfix.push_back(stack.back());
                stack.pop_back();
            }
        }

        //cout << "infix: ";
        //print_exp(infix);
        //cout << "stack: ";
        //print_exp(stack);
        //cout << "postfix: ";
        //print_exp(postfix);
        //cout << endl;
    }
    while (!stack.empty()) {
        postfix.push_back(stack.back());
        stack.pop_back();

        //cout << "infix: ";
        //print_exp(infix);
        //cout << "stack: ";
        //print_exp(stack);
        //cout << "postfix: ";
        //print_exp(postfix);
        //cout << endl;
    }
    //cout << "postfix: ";
    //print_exp(postfix);
    return postfix;
}

void SemanticAnalyser::delete_statement(BStatement* statement){
    if (Assign* assign = dynamic_cast<Assign*>(statement))
        delete assign;
    else if (Read* read = dynamic_cast<Read*>(statement))
        delete read;
    else if (Data* data = dynamic_cast<Data*>(statement))
        delete data;
    else if (Print* print = dynamic_cast<Print*>(statement))
        delete print;
    else if (Goto* go = dynamic_cast<Goto*>(statement))
        delete go;
    else if (If* ift = dynamic_cast<If*>(statement))
        delete ift;
    else if (For* loop = dynamic_cast<For*>(statement))
        delete loop;
    else if (Next* next = dynamic_cast<Next*>(statement))
        delete next;
    else if (Dim* dim = dynamic_cast<Dim*>(statement))
        delete dim;
    else if (Def* def = dynamic_cast<Def*>(statement))
        delete def;
    else if (Gosub* gosub = dynamic_cast<Gosub*>(statement))
        delete gosub;
    else if (Return* ret = dynamic_cast<Return*>(statement))
        delete ret;
    else if (End* end = dynamic_cast<End*>(statement))
        delete end;
}
#include <iostream>
#include <string>
#include <string.h>

#include "lexic.hpp"
#include "syntax.hpp"
#include "semantic.hpp"
#include "generation.hpp"
#include "ASCIIClassifier.hpp"
#include "LexicalAnalyser.hpp"
#include "SyntaxAnalyser.hpp"
#include "SemanticAnalyser.hpp"
#include "CodeGenerator.hpp"

#include "test.hpp"

using namespace std;


void ascii_test(ifstream& file) {
    using namespace lexic;
    
    ASCIIClassifier ac(file);

    while (!file.eof()) {
        ascii_character c = ac.get_next();
        cout << c.pos.position_str() << "\t" << c.character << "\t[" << ascii2name(c.type) << "]" << endl;
    }
}

void lex_test(ifstream& file) {
    using namespace lexic;

    LexicalAnalyser lex(file);

    while (true) {
        token s = lex.get_next();

        if (s.value == "") break;
            cout << "(" << s.pos.line << "," << s.pos.column << ")\t[" << type2name(s.type) << "] " << s.value << endl;
    }
}

string tab = "";

void print_var(syntax::Var* var) {
    cout << tab << "VAR[ " << var->get_identifier() <<" ]";
}

void print_num(syntax::Num* num) {
    cout << tab << "NUM[ " << num->get_value() <<" ]";
}

void print_call(syntax::Call* call) {
    cout << tab << "CALL" << endl;
    tab.push_back('\t');
    cout << tab << "FN [" << call->get_identifier() << "]" << endl;
    for (auto arg : call->get_args()) {
        print_exp(arg);
        cout << endl;
    }
}

void print_op(syntax::Operator* op) {
    cout << tab << "OP [ " << op->get_symbol() << " ]";
}

void print_op(syntax::If::cmp op) {
    switch (op) {
        case syntax::If::EQL: cout << "="; break;
        case syntax::If::NEQ: cout << "<>"; break;
        case syntax::If::GTN: cout << ">"; break;
        case syntax::If::LTN: cout << "<"; break;
        case syntax::If::GEQ: cout << ">="; break;
        case syntax::If::LEQ: cout << "<="; break;
    }
}

void print_exp(syntax::Exp* e) {
    cout << tab << "EXP ";
    cout << (e->is_negative()? "-" : "");
    tab.push_back('\t');
    cout << "\n";
    if (e->get_operands().at(0)->get_eb_type() == syntax::Eb::NUM) {
        print_num(dynamic_cast<syntax::Num*>(e->get_operands().at(0)));
    }
    else if (e->get_operands().at(0)->get_eb_type() == syntax::Eb::VAR) {
        print_var(dynamic_cast<syntax::Var*>(e->get_operands().at(0)));
    }
    else if (e->get_operands().at(0)->get_eb_type() == syntax::Eb::CALL) {
        print_call(dynamic_cast<syntax::Call*>(e->get_operands().at(0)));
    }
    else if (e->get_operands().at(0)->get_eb_type() == syntax::Eb::EXP) {
        print_exp(dynamic_cast<syntax::Exp*>(e->get_operands().at(0)));
    }

    for (int i = 0; i < e->get_operators().size(); i++) {
        cout << "\n";
        print_op(e->get_operators().at(i));

        cout << "\n";
        if (e->get_operands().at(i + 1)->get_eb_type() == syntax::Eb::NUM) {
            print_num(dynamic_cast<syntax::Num*>(e->get_operands().at(i + 1)));
        }
        else if (e->get_operands().at(i + 1)->get_eb_type() == syntax::Eb::VAR) {
            print_var(dynamic_cast<syntax::Var*>(e->get_operands().at(i + 1)));
        }
        else if (e->get_operands().at(i + 1)->get_eb_type() == syntax::Eb::CALL) {
            print_call(dynamic_cast<syntax::Call*>(e->get_operands().at(i + 1)));
        }
        else if (e->get_operands().at(i + 1)->get_eb_type() == syntax::Eb::EXP) {
            print_exp(dynamic_cast<syntax::Exp*>(e->get_operands().at(i + 1)));
        }

        cout << " ";
    }
    tab.pop_back();
    cout << endl;
}

void print_pitem(syntax::Pitem* pitem) {
    cout << tab << "PITEM";
    if (pitem->has_exp()) {
        tab.push_back('\t');
        cout << "\n";
        print_exp(pitem->get_exp());
        tab.pop_back();
        cout << "\n" << tab;
    }
    else {
        cout << "[ ";
        cout << pitem->get_str();
        cout << "]";
    }
}

void stx_test(ifstream& file) {
    using namespace std;
    using namespace syntax;

    SyntaxAnalyser stx(file);

    syntax::BStatement* command;

    while (true) {
        command = stx.get_next();

        if (command == nullptr)
            break;

        if (Assign* assign = dynamic_cast<Assign*>(command)) {
            cout << assign->get_index() << " ASSIGN ";
            tab.push_back('\t');
            cout << endl;
            print_var(assign->get_variable());
            cout << endl;
            print_exp(assign->get_expression());
            tab.pop_back();
            cout << endl;
        }
        else if (Read* read = dynamic_cast<Read*>(command)) {
            cout << read->get_index() << " READ " << endl;
            tab.push_back('\t');
            for (auto var : read->get_variables()) {
                if (var != *read->get_variables().begin())
                    cout << endl;
                print_var(var);
            }
            tab.pop_back();
            cout << endl;
        }
        else if (Data* data = dynamic_cast<Data*>(command)) {
            cout << data->get_index() << " DATA" << endl;
            tab.push_back('\t');
            for (auto num : data->get_values()) {
                if (num != *data->get_values().begin())
                    cout << endl;
                print_num(num);
            }
            tab.pop_back();
            cout << endl;
        }
        else if (Print* print = dynamic_cast<Print*>(command)) {
            cout << print->get_index() << " PRINT" << endl;
            tab.push_back('\t');
            for (auto pitem : print->get_pitems()) {
                if (pitem != *print->get_pitems().begin())
                    cout << endl;
                print_pitem(pitem);
            }
            tab.pop_back();
            cout << endl;
        }
        else if (Goto* go = dynamic_cast<Goto*>(command)) {
            cout << go->get_index() << " GOTO [ " << go->get_destination() << " ]" << endl;
        }
        else if (If* ift = dynamic_cast<If*>(command)) {
            cout << ift->get_index() << " IF";
            tab.push_back('\t');
            print_exp(ift->get_left());
            cout << endl;
            cout << tab << "OP [ ";
            print_op(ift->get_op());
            cout << " ]" << endl;
            print_exp(ift->get_right());
            cout << endl;
            cout << tab << "THEN [ " << ift->get_destination() << " ]" << endl;
            tab.pop_back();
        }
        else if (For* loop = dynamic_cast<For*>(command)) {
            cout << loop->get_index() << " FOR" << endl;
            tab.push_back('\t');
            print_var(loop->get_iterator());
            cout << endl;
            print_exp(loop->get_init());
            cout << endl;
            cout << tab << "TO" << endl;
            tab.push_back('\t');
            print_exp(loop->get_stop());
            tab.pop_back();
            cout << endl;
            cout << tab << "STEP" << endl;
            tab.push_back('\t');
            print_exp(loop->get_step());
            tab.pop_back();
            cout << endl;
        }
        else if (Next* next = dynamic_cast<Next*>(command)) {
            cout << next->get_index() << " NEXT" << endl;
            tab.push_back('\t');
            print_var(next->get_iterator());
            tab.pop_back();
            cout << endl;
        }
        else if (Dim* dim = dynamic_cast<Dim*>(command)) {
            cout << dim->get_index() << " DIM" << endl;
            tab.push_back('\t');
            for (auto array : dim->get_arrays()) {
                cout << tab << " ARRAY [ " << array->get_identifier();
                for (auto dimension : array->get_dimensions())
                    cout << "[" << dimension << "]";
                cout << " ]" << endl;
            }
            tab.pop_back();
            cout << endl;
        }
        else if (Def* def = dynamic_cast<Def*>(command)) {
            cout << def->get_index() << " DEF" << endl;
            tab.push_back('\t');
            cout << tab << "FN [ " << def->get_identifier() << " ]" << endl;
            for (auto param : def->get_parameters()) {
                if (param != *def->get_parameters().begin())
                    cout << endl;
                print_var(param);
            }
            cout << endl;
            print_exp(def->get_exp());
            tab.pop_back();
        }
        else if (Gosub* gosub = dynamic_cast<Gosub*>(command)) {
            cout << gosub->get_index() << " GOSUB [ " << gosub->get_destination() << " ]" << endl;
        }
        else if (Return* ret = dynamic_cast<Return*>(command)) {
            cout << ret->get_index() << " RETURN" << endl;
        }
        else if (Rem* rem = dynamic_cast<Rem*>(command)) {
            cout << rem->get_index() << " REMARK" << endl;
        }
        else if (End* end = dynamic_cast<End*>(command)) {
            cout << end->get_index() << " END" << endl;
        }
        else {
            cout << "É outra coisa" << endl;
        }
    }
}

string ascii2name(lexic::ascii_type t) {
    switch (t) {
        case lexic::ascii_type::UNKNOWN: return "UNKNOWN";
        case lexic::ascii_type::DIGIT: return "DIGIT";
        case lexic::ascii_type::LETTER: return "LETTER";
        case lexic::ascii_type::SPECIAL: return "SPECIAL";
        case lexic::ascii_type::DELIMITER: return "DELIMITER";
    }
}

string type2name(lexic::type t) {
    switch(t) {
        case lexic::type::STR: return "STR";
        case lexic::type::INT: return "INT";
        case lexic::type::IDN: return "IDN";
        case lexic::type::LET: return "LET";
        case lexic::type::FN: return "FN";
        case lexic::type::DEF: return "DEF";
        case lexic::type::READ: return "READ";
        case lexic::type::DATA: return "DATA";
        case lexic::type::PRINT: return "PRINT";
        case lexic::type::GO: return "GO";
        case lexic::type::TO: return "TO";
        case lexic::type::GOTO: return "GOTO";
        case lexic::type::IF: return "IF";
        case lexic::type::THEN: return "THEN";
        case lexic::type::FOR: return "FOR";
        case lexic::type::STEP: return "STEP";
        case lexic::type::NEXT: return "NEXT";
        case lexic::type::DIM: return "DIM";
        case lexic::type::GOSUB: return "GOSUB";
        case lexic::type::RETURN: return "RETURN";
        case lexic::type::REM: return "REM";
        case lexic::type::END: return "END";
        case lexic::type::ADD: return "ADD";
        case lexic::type::SUB: return "SUB";
        case lexic::type::MUL: return "MUL";
        case lexic::type::DIV: return "DIV";
        case lexic::type::POW: return "POW";
        case lexic::type::EQL: return "EQL";
        case lexic::type::NEQ: return "NEQ";
        case lexic::type::LTN: return "LTN";
        case lexic::type::GTN: return "GTN";
        case lexic::type::LEQ: return "LEQ";
        case lexic::type::GEQ: return "GEQ";
        case lexic::type::COM: return "COM";
        case lexic::type::PNT: return "PNT";
        case lexic::type::PRO: return "PRO";
        case lexic::type::PRC: return "PRC";
        case lexic::type::DQT: return "DQT";
        case lexic::type::EXD: return "EXD";
        case lexic::type::FNSIN: return "FNSIN";
        case lexic::type::FNCOS: return "FNCOS";
        case lexic::type::FNTAN: return "FNTAN";
        case lexic::type::FNATN: return "FNATN";
        case lexic::type::FNEXP: return "FNEXP";
        case lexic::type::FNABS: return "FNABS";
        case lexic::type::FNLOG: return "FNLOG";
        case lexic::type::FNSQR: return "FNSQR";
        case lexic::type::FNINT: return "FNINT";
        case lexic::type::FNRND: return "FNRND";
        case lexic::type::CMT: return "CMT";
        case lexic::type::EoF: return "EOF";
    }
}
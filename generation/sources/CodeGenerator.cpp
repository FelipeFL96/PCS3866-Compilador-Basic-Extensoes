#include <iostream>

#include "syntax.hpp"
#include "generation.hpp"

#include "CodeGenerator.hpp"

#define STACK_SIZE 256

using namespace std;
using namespace generation;

bool found_div = false;
bool found_pow = false;

CodeGenerator::CodeGenerator(string& input_file, string& output_file, semantic::SymbolTable& symb_table):
    input_file(input_file), symb_table(symb_table)
{
    output.open(output_file);
    if (!output.is_open())
        throw generation_exception("Não foi possível abrir o arquivo '" + output_file + "' para saída");
}

CodeGenerator::~CodeGenerator() {
    output.close();
}

void CodeGenerator::generate_header(int first_index) {
    output << "/* BASIC COMPILER */" << endl;
    output << "/* source: " << input_file << " */" << endl;
    output << ".global main" << endl;
    output << endl;

    output << "main: " << endl;
    output << "\tLDR      r12, =variables" << endl;
    output << "\tLDR      r11, =exe_stack" << endl;
    output << "\tLDR      sp,  =exp_stack" << endl;
    output << "\tB        L" << first_index << endl;
    output << endl;
}

void CodeGenerator::generate_variables() {
    install_predef();
    output << "variables: " << endl;
    output << "\t.space " << symb_table.total_variable_size() << endl;
    output << endl;
    // Espaço para a pilha
    output << "\t.space " << STACK_SIZE << endl;
    output << "exe_stack: " << endl;
    output << "\t.space " << STACK_SIZE << endl;
    output << "exp_stack: " << endl;
    output << endl;
}

void CodeGenerator::generate(syntax::Assign* assign, vector<syntax::Elem*> exp, int next_index) {
    output << "L" << assign->get_index() << ":" << endl;
    generate_expression(exp);
    output << "\tSTR      r0, [r12, #" << 4 * symb_table.select_variable(assign->get_variable()) << "]" << endl;
    output << "\tB        L" << next_index << endl;
    output << endl;
}

void CodeGenerator::generate(syntax::Read* read, std::vector<pair<syntax::Var*, syntax::Num*>>& read_data, int next_index) {
    output << "L" << read->get_index() << ":" << endl;
    for (auto pair : read_data) {
        syntax::Var* var = get<0>(pair);
        syntax::Num* val = get<1>(pair);

        if (var->is_array()) {
            syntax::ArrayAccess* access = dynamic_cast<syntax::ArrayAccess*>(var);
            vector<syntax::Elem*> access_expression = access->get_processed_access_exps();

            generate_expression(access_expression);

            output << "\tMOV      r1, r0, LSL #2" << endl;
            output << "\tADD      r1, r1, #" << 4 * symb_table.select_variable(var) << endl;
            output << "\tMOV      r0, #" << val->get_value() << endl;
            output << "\tSTR      r0, [r12, r1]" << endl;
        }
        else {
            output << "\tMOV      r0, #" << val->get_value() << endl;
            output << "\tSTR      r0, [r12, #" << 4 * symb_table.select_variable(var) << "]" << endl;
        }
    }
    output << "\tB        L" << next_index << endl;
    output << endl;
}

void CodeGenerator::generate(syntax::Data* data, std::vector<pair<syntax::Var*, syntax::Num*>>& read_data, int next_index) {
    output << "L" << data->get_index() << ":" << endl;
    for (auto pair : read_data) {
        syntax::Var* var = get<0>(pair);
        syntax::Num* val = get<1>(pair);

        if (var->is_array()) {
            syntax::ArrayAccess* access = dynamic_cast<syntax::ArrayAccess*>(var);
            vector<syntax::Elem*> access_expression = access->get_processed_access_exps();

            generate_expression(access_expression);

            output << "\tMOV      r1, r0, LSL #2" << endl;
            output << "\tADD      r1, r1, #" << 4 * symb_table.select_variable(var) << endl;
            output << "\tMOV      r0, #" << val->get_value() << endl;
            output << "\tSTR      r0, [r12, r1]" << endl;
        }
        else {
            output << "\tMOV      r0, #" << val->get_value() << endl;
            output << "\tSTR      r0, [r12, #" << 4 * symb_table.select_variable(var) << "]" << endl;
        }
    }
    output << "\tB        L" << next_index << endl;
    output << endl;
}

void CodeGenerator::generate(syntax::Goto* go, int destination) {
    output << "L" << go->get_index() << ":" << endl;
    output << "\tB        L" << destination << endl;
    output << endl;
}

void CodeGenerator::generate(syntax::If* ift, std::vector<syntax::Elem*> left, std::vector<syntax::Elem*> right, int destination, int next_index) {
    output << "L" << ift->get_index() << ":" << endl;
    generate_expression(left);
    output << "\tSTMFD    sp!, {r0}" << endl;
    generate_expression(right);
    output << "\tLDMFD    sp!, {r1}" << endl;
    output << "\tCMP      r1, r0" << endl;
    switch (ift->get_op()) {
        case syntax::If::EQL:
            output << "\tBEQ      " << "L" << destination << endl;
            break;
        case syntax::If::NEQ:
            output << "\tBNE      " << "L" << destination << endl;
            break;
        case syntax::If::GTN:
            output << "\tBGT      " << "L" << destination << endl;
            break;
        case syntax::If::LTN:
            output << "\tBLT      " << "L" << destination << endl;
            break;
        case syntax::If::GEQ:
            output << "\tBGE      " << "L" << destination << endl;
            break;
        case syntax::If::LEQ:
            output << "\tBLE      " << "L" << destination << endl;
            break;
    }
    output << "\tB        L" << next_index << endl;
    output << endl;
}

void CodeGenerator::generate(syntax::For* loop, vector<syntax::Elem*> init, vector<syntax::Elem*> stop,
            vector<syntax::Elem*> step, int index_inside_loop, int index_outside_loop) {
    output << "L" << loop->get_index() << ":" << endl;

    // Inicialização do iterador
    generate_expression(init);
    output << "\tSTR      r0, [r12, #" << 4 * symb_table.select_variable(loop->get_iterator()) << "]" << endl;
    output << "\tB        L" << loop->get_index() << ".COMP" << endl;
    output << endl;

    // Incremento do iterador
    output << "L" << loop->get_index() << ".STEP:" << endl;
    generate_expression(step);
    output << "\tLDR      r1, [r12, #" << 4 * symb_table.select_variable(loop->get_iterator()) << "]" << endl;
    output << "\tADD      r0, r1, r0" << endl;
    output << "\tSTR      r0, [r12, #" << 4 * symb_table.select_variable(loop->get_iterator()) << "]" << endl;
    output << endl;

    // Comparação com a condição de parada
    output << "L" << loop->get_index() << ".COMP:" << endl;
    generate_expression(stop);
    output << "\tLDR      r1, [r12, #" << 4 * symb_table.select_variable(loop->get_iterator()) << "]" << endl;
    output << "\tCMP      r1, r0" << endl;
    output << "\tBGE      L" << index_outside_loop << endl;
    output << "\tB        L" << index_inside_loop << endl;
    output << endl;
}

void CodeGenerator::generate(syntax::Next* next) {
    output << "L" << next->get_index() << ":" << endl;
    output << "\tB        L" << next->get_loop()->get_index() << ".STEP" << endl;
    output << endl;
}

void CodeGenerator::generate(syntax::Def* def, std::vector<syntax::Elem*>& exp) {
    output << def->get_identifier() << ":" << endl;

    auto parameters = def->get_parameters();
    while (!parameters.empty()) {
        auto param = parameters.back();
        output << "\tLDMFD    sp!, {r1}" << endl;
        output << "\tSTR      r1, [r12, #"
            << 4 * symb_table.select_variable(param) << "]" << endl;
        parameters.pop_back();
    }

    generate_expression(exp);
    output << "\tMOV      pc, lr" << endl;
    output << endl;
}

void CodeGenerator::generate(syntax::Gosub* gosub, int destination) {
    output << "L" << gosub->get_index() << ":" << endl;
    output << "\tSTMFD    r11!, {lr}" << endl;
    output << "\tBL       L" << destination << endl;
    output << "\tLDMFD    r11!, {lr}" << endl;
    output << endl;
}

void CodeGenerator::generate(syntax::Return* ret) {
    output << "L" << ret->get_index() << ":" << endl;
    output << "\tMOV      pc, lr" << endl;
    output << endl;
}

void CodeGenerator::generate(syntax::End* end) {
    output << "L" << end->get_index() << ":" << endl;
    output << "\tB        L" << end->get_index() << endl;
    output << endl;
}

void CodeGenerator::generate_expression(vector<syntax::Elem*>& exp) {
    for (auto e : exp) {
        if (e->get_elem_type() == syntax::Elem::NUM) {
            output << "\tMOV      r1, #" << dynamic_cast<syntax::Num*>(e)->get_value() << endl;
            output << "\tSTMFD    sp!, {r1}" << endl;
        }
        else if (e->get_elem_type() == syntax::Elem::VAR) {
            syntax::Var* var = dynamic_cast<syntax::Var*>(e);

            if (var->is_array()) {
                syntax::ArrayAccess* access = dynamic_cast<syntax::ArrayAccess*>(var);
                output << "\tLDMFD    sp!, {r1}" << endl;
                output << "\tMOV      r1, r1, LSL #2" << endl;
                output << "\tADD      r1, r1, #" << 4 * symb_table.select_variable(var) << endl;
                output << "\tLDR      r1, [r12, r1]" << endl;
                output << "\tSTMFD    sp!, {r1}" << endl;
            }
            else {
                output << "\tLDR      r1, [r12, #" << 4 * symb_table.select_variable(var) << "]" << endl;
                output << "\tSTMFD    sp!, {r1}" << endl;
            }
        }
        else if (e->get_elem_type() == syntax::Elem::FUN) {
            output << "\tSTMFD    r11!, {lr}" << endl;
            output << "\tBL       " << dynamic_cast<syntax::Call*>(e)->get_identifier() << endl;
            output << "\tLDMFD    r11!, {lr}" << endl;
            output << "\tSTMFD    sp!, {r0}" << endl;
        }
        else if (e->is_operator()) {
            output << "\tLDMFD    sp!, {r2}" << endl;
            output << "\tLDMFD    sp!, {r1}" << endl;
            if (e->get_elem_type() == syntax::Elem::ADD) {
                output << "\tADD      r0, r1, r2" << endl;
            }
            else if (e->get_elem_type() == syntax::Elem::SUB) {
                output << "\tSUB      r0, r1, r2" << endl;
            }
            else if (e->get_elem_type() == syntax::Elem::MUL) {
                output << "\tMUL      r0, r1, r2" << endl;
            }
            else if (e->get_elem_type() == syntax::Elem::DIV) {
                found_div = true;
                output << "\tSTMFD    r11!, {lr}" << endl;
                output << "\tBL       sdiv" << endl;
                output << "\tLDMFD    r11!, {lr}" << endl;
            }
            else if (e->get_elem_type() == syntax::Elem::POW) {
                found_pow = true;
                output << "\tSTMFD    r11!, {lr}" << endl;
                output << "\tBL       pow" << endl;
                output << "\tLDMFD    r11!, {lr}" << endl;
            }
            output << "\tSTMFD    sp!, {r0}" << endl;
        }
    }
    output << "\tLDMFD    sp!, {r0}" << endl;
}

void CodeGenerator::install_predef() {
    if (found_div)
        install_sdiv();
    if (found_pow)
        install_pow();
}

/*
 * SDIV r0, r1, r2
 * r0 = r1 / r2
 * Divisão inteira
 * r0 = 0 se r1 = 0
 */
void CodeGenerator::install_sdiv() {
    output << "sdiv:" << endl;
    output << "\tMOV      r4, #0" << endl;
    output << "\tMOV      r5, #0" << endl;
    output << endl;
    output << "\tCMP      r2, #0" << endl;
    output << "\tBEQ      sdiv.end" << endl;
    output << "\tMOVLT    r4, #1" << endl;
    output << "\tRSBLT    r2, r2, #0" << endl;
    output << endl;
    output << "\tCMP      r1, #0" << endl;
    output << "\tMOVLT    r5, #1" << endl;
    output << "\tRSBLT    r1, r1, #0" << endl;
    output << "\tEOR      r4, r4, r5" << endl;
    output << endl;
    output << "\tMOV      r0, #0" << endl;
    output << "\tMOV      r3, #1" << endl;
    output << endl;
    output << "sdiv.start:" << endl;
    output << "\tCMP      r2, r1" << endl;
    output << "\tMOVLS    r2, r2, LSL #1" << endl;
    output << "\tMOVLS    r3, r3, LSL #1" << endl;
    output << "\tBLS      sdiv.start" << endl;
    output << endl;
    output << "sdiv.next:" << endl;
    output << "\tCMP       r1, r2" << endl;
    output << "\tSUBCS     r1, r1, r2" << endl;
    output << "\tADDCS     r0, r0, r3" << endl;
    output << "\tMOVS      r3, r3, LSR #1" << endl;
    output << "\tMOVCC     r2, r2, LSR #1" << endl;
    output << "\tBCC       sdiv.next" << endl;
    output << endl;
    output << "sdiv.end:" << endl;
    output << "\tCMP      r4, #1" << endl;
    output << "\tRSBEQ    r0, r0, #0" << endl;
    output << "\tMOV      pc, lr" << endl;
    output << endl;
}

/*
 * POW r0, r1, r2
 * r0 = r1 ^ r2
 * Potenciação simples de inteiros
 * r0 = 0 se r2 < 0
 */
void CodeGenerator::install_pow() {
    output << "pow:" << endl;
    output << "\tCMP      r1, #0" << endl;
    output << "\tMOVEQ    r0, #0" << endl;
    output << "\tBEQ      pow.end" << endl;
    output << "\t" << endl;
    output << "\tCMP      r2, #0" << endl;
    output << "\tMOVLT    r0, #0" << endl;
    output << "\tBEQ      pow.end" << endl;
    output << "\t" << endl;
    output << "\tMOV      r0, #1" << endl;
    output << "\tMOV      r3, r2" << endl;
    output << endl;
    output << "pow.loop:" << endl;
    output << "\tCMP      r3, #0" << endl;
    output << "\tBEQ      pow.end" << endl;
    output << "\tMOV      r4, r0" << endl;
    output << "\tMUL      r0, r4, r1" << endl;
    output << "\tSUB      r3, r3, #1" << endl;
    output << "\tB        pow.loop" << endl;
    output << endl;
    output << "pow.end:" << endl;
    output << "\tMOV      pc, lr" << endl;
    output << endl;
}

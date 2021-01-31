#include "lexic.hpp"
#include "syntax.hpp"

using namespace std;

string type2name(lexic::type t);
string ascii2name(lexic::ascii_type t);

void ascii_test(ifstream& file);
void lex_test(ifstream& file);
void stx_test(ifstream& file);

void print_var(syntax::Var* var);
void print_num(syntax::Num* num);
void print_call(syntax::Call* call);
void print_op(syntax::Operator* op);
void print_op(syntax::If::cmp op);
void print_exp(syntax::Exp* e);
void print_pitem(syntax::Pitem* pitem);

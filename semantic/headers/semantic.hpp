#ifndef SEMANTIC_HPP
#define SEMANTIC_HPP

#include <iostream>
#include <exception>
#include <utility>
#include <vector>

#include "syntax.hpp"

namespace semantic {

class SymbolTable {
    public:
    SymbolTable()
    {}

    int insert_variable(syntax::Var* v) {
        if (int var_index = select_variable(v) != 0) {
            return var_index;
        }

        v->set_index(index);
        variables.push_back(std::make_pair(v, index));
        return index++;
    }

    int insert_array(syntax::Array* a) {
        if (int var_index = select_variable(dynamic_cast<syntax::Var*>(a)) != 0)
            return var_index;

        a->set_index(index);
        variables.push_back(std::make_pair(a, index));
        int var_index = index;

        int size = 1;
        for (int d : a->get_dimensions())
            size *= d;
        index += size;

        return var_index;
    }

    int select_variable(syntax::Var* v) {
        if (variables.empty())
            return 0;

        for (auto pair : variables) {
            if (std::get<0>(pair)->get_identifier() == v->get_identifier()) {
                return std::get<1>(pair);
            }
        }

        return 0;
    }

    syntax::Var* pointer_to_variable(syntax::Var* v) {
        if (variables.empty())
            return nullptr;

        for (auto pair : variables) {
            if (std::get<0>(pair)->get_identifier() == v->get_identifier()) {
                return std::get<0>(pair);
            }
        }

        return nullptr;
    }

    int total_variable_size() {
        int total_size = 4;
        for (auto pair : variables) {
            total_size += std::get<0>(pair)->get_size();
        }
        return total_size;
    }

    void print_variables() {
        for (int i = 0; i < variables.size(); i++) {
            std::cout << "[" << std::get<1>(variables.at(i)) << "] " << std::get<0>(variables.at(i))->get_identifier() << std::endl;
        }
    }

    bool insert_function(syntax::Def* f) {
         if (select_function(f))
            return false;

        functions.push_back(f);
        return true;
    }

    syntax::Def* select_function(syntax::Call* c) {
        if (functions.empty())
            return nullptr;

        for (auto fn : functions) {
            if (fn->get_identifier() == c->get_identifier())
                return fn;
        }

        return nullptr;
    }

    syntax::Def* select_function(syntax::Def* f) {
        if (functions.empty())
            return nullptr;

        for (auto fn : functions) {
            if (fn->get_identifier() == f->get_identifier())
                return fn;
        }

        return nullptr;
    }
    
    private:
    int index = 1;
    std::vector<std::pair<syntax::Var*, int>> variables;
    std::vector<syntax::Def*> functions;
};

class semantic_exception: public std::exception {
    public:
        semantic_exception(lexic::position& loc, const std::string error_message)
            : loc(loc), error_message(error_message) {
            exception();
        }

        std::string message() {
            return loc.position_str() + ": " + error_message;
        }

    private:
        lexic::position& loc;
        const std::string error_message;
};

} // namespace semantic

#endif // SEMANTIC_HPP
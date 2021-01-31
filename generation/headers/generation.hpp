#ifndef GENERATION_HPP
#define GENERATION_HPP

#include <exception>

#include "lexic.hpp"

namespace generation {

class generation_exception: public std::exception {
    public:
        generation_exception(lexic::position* loc, const std::string error_message)
            : loc(loc), error_message(error_message) {
            exception();
        }

        generation_exception(const std::string error_message)
            : error_message(error_message) {
            exception();
        }

        std::string message() {
            if (loc)
                return loc->position_str() + ": " + error_message;
            else
                return error_message;
        }

    private:
        lexic::position* loc;
        const std::string error_message;
};

} // namespace generation

#endif // GENERATION_HPP
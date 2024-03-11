#include "loc.h"

#include <iostream>

namespace H {

int num_errors = 0;

std::ostream& Loc::err() const {
    ++num_errors;
    return std::cerr << (*this) << ": error: ";
}

std::ostream& operator<<(std::ostream& o, const Pos& pos) {
    return o << pos.row << ":" << pos.col;
}

std::ostream& operator<<(std::ostream& o, const Loc& loc) {
    o << loc.file << ":" << loc.begin;
    if (loc.begin.row != loc.finish.row) {
        o << "-" << loc.finish;
    } else {
        if (loc.begin.col != loc.finish.col)
            o << "-" << loc.finish.col;
    }

    return o;
}

}

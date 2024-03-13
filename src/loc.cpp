#include "loc.h"

#include <iostream>

namespace H {

int num_errors = 0;

std::ostream& Loc::err() {
    ++num_errors;
    this->begin = Pos(this->begin.row, this->begin.col);
    return std::cerr << "\033[1;31m" << (*this) << ": error: ";
}

std::string Loc::endErr() const { //reset error encoding
    return "\033[0m\n";
}

std::ostream& operator<<(std::ostream& o, const Pos& pos) {
    return o << pos.row << ":" << pos.col;
}

std::ostream& operator<<(std::ostream& o, const Loc& loc) {
    o << loc.file << ":" << loc.begin;
    //if (loc.begin.row != loc.finish.row) {
    //    o << "-" << loc.finish;
    //} else {
    //    if (loc.begin.col != loc.finish.col)
    //        o << "-" << loc.finish.col;
    // }

    return o;
}

}

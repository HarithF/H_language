#ifndef PROG_LOC_H
#define PROG_LOC_H

#include <ostream>

namespace H {

extern int num_errors;

struct Pos {
    Pos() = default;
    Pos(int row, int col)
        : row(row)
        , col(col)
    {}

    int row = -1;
    int col = -1;
};

struct Loc {
    Loc() = default;
    Loc(const char* file, Pos begin, Pos finish)
        : file(file)
        , begin(begin)
        , finish(finish)
    {}
    Loc(const char* file, Pos pos)
        : Loc(file, pos, pos)
    {}

    const char* file = nullptr;
    Pos begin;
    Pos finish;

    std::ostream& err(int offset=0);
    std::string endErr() const;
};

std::ostream& operator<<(std::ostream&, const Pos&);
std::ostream& operator<<(std::ostream&, const Loc&);

}

#endif

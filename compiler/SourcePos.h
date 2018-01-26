#pragma once

class SourcePos {
public:
    int line;
    int column;

    SourcePos(int line, int column) : line(line), column(column) { }
};

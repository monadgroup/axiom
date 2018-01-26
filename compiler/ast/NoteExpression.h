#pragma once

namespace MaximAst {

    class NoteExpression : public Expression {
    public:
        int note;

        NoteExpression(int note, SourcePos start, SourcePos end) : Expression(start, end), note(note) { }
    };

}

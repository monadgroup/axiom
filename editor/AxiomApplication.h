#pragma once

#include <QApplication>
#include <optional>

#include "compiler/interface/Jit.h"

namespace AxiomModel {
    class Project;
}

class AxiomApplication : public QApplication {
public:
    static AxiomApplication main;

    AxiomApplication();

    MaximCompiler::Jit *jit() { return &*_jit; }

private:
    std::optional<MaximCompiler::Jit> _jit;
};

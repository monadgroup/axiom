#pragma once

#include <QApplication>
#include <optional>

namespace AxiomModel {
    class Project;
}

class AxiomApplication : public QApplication {
public:
    static AxiomApplication main;

    AxiomApplication();
};

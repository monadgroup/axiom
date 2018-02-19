#pragma once

#include <QtCore/QObject>

#include "Node.h"
#include "compiler/runtime/CustomNode.h"

namespace AxiomModel {

    class CustomNode : public Node {
    Q_OBJECT

    public:
        CustomNode(Schematic *parent, QString name, QPoint pos, QSize);

        std::unique_ptr<GridItem> clone(GridSurface *newParent, QPoint newPos, QSize newSize) const override;

        MaximRuntime::CustomNode *runtime() override { return &_runtime; }

        QString code() const { return m_code; }

    public slots:

        void setCode(const QString &code);

        void recompile();

    signals:

        void codeChanged(QString newCode);

        void compileFailed(const MaximRuntime::ErrorLog &log);

        void compileSucceeded();

    private slots:

        void controlAdded(MaximRuntime::HardControl *control);

    private:
        QString m_code = "";
        MaximRuntime::CustomNode _runtime;
    };

}

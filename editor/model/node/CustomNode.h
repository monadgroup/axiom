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

        MaximRuntime::CustomNode *runtime() override { return _runtime; }

        void attachRuntime(MaximRuntime::CustomNode *runtime);

        void createAndAttachRuntime(MaximRuntime::Surface *surface) override;

        QString code() const { return m_code; }

    public slots:

        void setCode(const QString &code);

        void parseCode();

        void recompile();

        void serialize(QDataStream &stream, QPoint offset) const override;

        void deserialize(QDataStream &stream, QPoint offset) override;

    signals:

        void codeChanged(QString newCode);

        void parseFailed(const MaximRuntime::ErrorLog &log);

        void compileFailed(const MaximRuntime::ErrorLog &log);

        void parseSucceeded();

        void compileSucceeded();

        void compileFinished();

    private slots:

        void controlAdded(MaximRuntime::Control *control);

        void finishedCodegen();

    private:
        QString lastCode = "";
        QString m_code = "";
        MaximRuntime::CustomNode *_runtime = nullptr;
        //std::unique_ptr<MaximRuntime::CustomNode> _runtime;
    };

}

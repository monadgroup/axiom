#pragma once

#include "Node.h"
#include "NodeSurface.h"

namespace AxiomModel {

    class CustomNode : public Node {
    public:
        CustomNode(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name, const QUuid &controlsUuid, QString code, ModelRoot *root);

        static std::unique_ptr<CustomNode> deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name, const QUuid &controlsUuid, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        const QString &code() const { return _code; }

    private:
        QString _code;
    };

}

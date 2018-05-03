#pragma once

#include "../ModelObject.h"
#include "../grid/GridItem.h"

namespace AxiomModel {

    class Node : public GridItem, public ModelObject {
    public:
        Node(ModelType type, const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, ModelRoot *root);

        void deserialize(QDataStream &stream, QPoint &pos, QSize &size, bool &selected) const;

        void serialize(QDataStream &stream) const override;
    };

}

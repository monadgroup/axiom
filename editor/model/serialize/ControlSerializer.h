#pragma once

#include <QtCore/QDataStream>
#include <memory>

#include "../ConnectionWire.h"

namespace AxiomModel {

    class ModelRoot;
    class Control;
    class ExtractControl;
    class MidiControl;
    class NumControl;
    class PortalControl;
    class GraphControl;
    class ReferenceMapper;

    namespace ControlSerializer {
        void serialize(Control *control, QDataStream &stream);

        std::unique_ptr<Control> deserialize(QDataStream &stream, uint32_t version, const QUuid &uuid,
                                             const QUuid &parentUuid, ReferenceMapper *ref, ModelRoot *root);

        void serializeExtract(ExtractControl *control, QDataStream &stream);

        std::unique_ptr<ExtractControl> deserializeExtract(QDataStream &stream, uint32_t version, const QUuid &uuid,
                                                           const QUuid &parentUuid, QPoint pos, QSize size,
                                                           bool selected, QString name, bool showName,
                                                           QUuid exposerUuid, QUuid exposingUuid,
                                                           ConnectionWire::WireType wireType, ReferenceMapper *ref,
                                                           ModelRoot *root);

        void serializeMidi(MidiControl *control, QDataStream &stream);

        std::unique_ptr<MidiControl> deserializeMidi(QDataStream &stream, uint32_t version, const QUuid &uuid,
                                                     const QUuid &parentUuid, QPoint pos, QSize size, bool selected,
                                                     QString name, bool showName, QUuid exposerUuid, QUuid exposingUuid,
                                                     ReferenceMapper *ref, ModelRoot *root);

        void serializeNum(NumControl *control, QDataStream &stream);

        std::unique_ptr<NumControl> deserializeNum(QDataStream &stream, uint32_t version, const QUuid &uuid,
                                                   const QUuid &parentUuid, QPoint pos, QSize size, bool selected,
                                                   QString name, bool showName, QUuid exposerUuid, QUuid exposingUuid,
                                                   ReferenceMapper *ref, ModelRoot *root);

        void serializePortal(PortalControl *control, QDataStream &stream);

        std::unique_ptr<PortalControl> deserializePortal(QDataStream &stream, uint32_t version, const QUuid &uuid,
                                                         const QUuid &parentUuid, QPoint pos, QSize size, bool selected,
                                                         QString name, bool showName, QUuid exposerUuid,
                                                         QUuid exposingUuid, ConnectionWire::WireType wireType,
                                                         ReferenceMapper *ref, ModelRoot *root);

        void serializeGraph(GraphControl *control, QDataStream &stream);

        std::unique_ptr<GraphControl> deserializeGraph(QDataStream &stream, uint32_t version, const QUuid &uuid,
                                                       const QUuid &parentUuid, QPoint pos, QSize size, bool selected,
                                                       QString name, bool showName, QUuid exposerUuid,
                                                       QUuid exposingUuid, ReferenceMapper *ref, ModelRoot *root);
    }
}

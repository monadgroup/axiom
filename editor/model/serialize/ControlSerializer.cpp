#include "ControlSerializer.h"

#include "../../util.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../ReferenceMapper.h"
#include "../objects/Control.h"
#include "../objects/ControlSurface.h"
#include "../objects/ExtractControl.h"
#include "../objects/GraphControl.h"
#include "../objects/GroupSurface.h"
#include "../objects/MidiControl.h"
#include "../objects/Node.h"
#include "../objects/NumControl.h"
#include "../objects/PortalControl.h"
#include "../objects/RootSurface.h"
#include "ValueSerializer.h"

using namespace AxiomModel;

void ControlSerializer::serialize(AxiomModel::Control *control, QDataStream &stream) {
    stream << (uint8_t) control->controlType();
    stream << control->pos();
    stream << control->size();
    stream << control->isSelected();
    stream << control->name();
    stream << control->showName();
    stream << control->exposerUuid();
    stream << control->exposingUuid();

    if (auto extract = dynamic_cast<ExtractControl *>(control))
        serializeExtract(extract, stream);
    else if (auto midi = dynamic_cast<MidiControl *>(control))
        serializeMidi(midi, stream);
    else if (auto num = dynamic_cast<NumControl *>(control))
        serializeNum(num, stream);
    else if (auto portal = dynamic_cast<PortalControl *>(control))
        serializePortal(portal, stream);
    else if (auto graph = dynamic_cast<GraphControl *>(control))
        serializeGraph(graph, stream);
    else
        unreachable;
}

std::unique_ptr<Control> ControlSerializer::deserialize(QDataStream &stream, uint32_t version, const QUuid &uuid,
                                                        const QUuid &parentUuid, AxiomModel::ReferenceMapper *ref,
                                                        AxiomModel::ModelRoot *root) {
    uint8_t controlTypeInt;
    stream >> controlTypeInt;

    QPoint pos;
    stream >> pos;

    QSize size;
    stream >> size;

    bool selected;
    stream >> selected;

    QString name;
    stream >> name;

    bool showName;
    stream >> showName;

    QUuid maybeExposerUuid;
    stream >> maybeExposerUuid;

    // The exposer should only be set if the actual control exists in this deserialization.
    // Since the exposer control is normally after the exposed one (but it doesn't have to be!) we can't just
    // check if its UUID is valid in the ReferenceMapper, instead we need to see if the node it would exist on is
    // valid. We do that here by walking up trying to find a GroupNode, and then checking to see if it's a valid
    // reference.
    QUuid exposerUuid;
    if (ref->isValid(maybeExposerUuid)) {
        // hot path if the exposer control happens to already exist
        exposerUuid = ref->mapUuid(maybeExposerUuid);
    } else if (!maybeExposerUuid.isNull()) {
        auto controlSurface = findMaybe(root->controlSurfaces(), parentUuid);
        if (controlSurface) {
            auto parentNode = findMaybe(root->nodes(), (*controlSurface)->parentUuid());
            if (parentNode) {
                auto groupSurface = findMaybe<GroupSurface *>(root->nodeSurfaces(), (*parentNode)->parentUuid());
                if (groupSurface && ref->isValid((*groupSurface)->parentUuid())) {
                    exposerUuid = ref->mapUuid(maybeExposerUuid);
                }
            }
        }
    }

    QUuid exposingUuid;
    stream >> exposingUuid;
    exposingUuid = ref->mapUuid(exposingUuid);

    switch ((Control::ControlType) controlTypeInt) {
    case Control::ControlType::NUM_SCALAR:
        return deserializeNum(stream, version, uuid, parentUuid, pos, size, selected, std::move(name), showName,
                              exposerUuid, exposingUuid, ref, root);
    case Control::ControlType::MIDI_SCALAR:
        return deserializeMidi(stream, version, uuid, parentUuid, pos, size, selected, std::move(name), showName,
                               exposerUuid, exposingUuid, ref, root);
    case Control::ControlType::NUM_EXTRACT:
        return deserializeExtract(stream, version, uuid, parentUuid, pos, size, selected, std::move(name), showName,
                                  exposerUuid, exposingUuid, ConnectionWire::WireType::NUM, ref, root);
    case Control::ControlType::MIDI_EXTRACT:
        return deserializeExtract(stream, version, uuid, parentUuid, pos, size, selected, std::move(name), showName,
                                  exposerUuid, exposingUuid, ConnectionWire::WireType::MIDI, ref, root);
    case Control::ControlType::NUM_PORTAL:
        return deserializePortal(stream, version, uuid, parentUuid, pos, size, selected, std::move(name), showName,
                                 exposerUuid, exposingUuid, ConnectionWire::WireType::NUM, ref, root);
    case Control::ControlType::MIDI_PORTAL:
        return deserializePortal(stream, version, uuid, parentUuid, pos, size, selected, std::move(name), showName,
                                 exposerUuid, exposingUuid, ConnectionWire::WireType::MIDI, ref, root);
    case Control::ControlType::GRAPH:
        return deserializeGraph(stream, version, uuid, parentUuid, pos, size, selected, std::move(name), showName,
                                exposerUuid, exposingUuid, ref, root);
    default:
        unreachable;
    }
}

void ControlSerializer::serializeExtract(AxiomModel::ExtractControl *control, QDataStream &stream) {
    stream << control->activeSlots();
}

std::unique_ptr<ExtractControl> ControlSerializer::deserializeExtract(
    QDataStream &stream, uint32_t version, const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
    bool selected, QString name, bool showName, QUuid exposerUuid, QUuid exposingUuid,
    AxiomModel::ConnectionWire::WireType wireType, AxiomModel::ReferenceMapper *ref, AxiomModel::ModelRoot *root) {
    ExtractControl::ActiveSlotFlags activeSlots;
    stream >> activeSlots;
    return ExtractControl::create(uuid, parentUuid, pos, size, selected, std::move(name), showName, exposerUuid,
                                  exposingUuid, wireType, activeSlots, root);
}

void ControlSerializer::serializeMidi(AxiomModel::MidiControl *control, QDataStream &stream) {}

std::unique_ptr<MidiControl> ControlSerializer::deserializeMidi(QDataStream &stream, uint32_t version,
                                                                const QUuid &uuid, const QUuid &parentUuid, QPoint pos,
                                                                QSize size, bool selected, QString name, bool showName,
                                                                QUuid exposerUuid, QUuid exposingUuid,
                                                                AxiomModel::ReferenceMapper *ref,
                                                                AxiomModel::ModelRoot *root) {
    return MidiControl::create(uuid, parentUuid, pos, size, selected, std::move(name), showName, exposerUuid,
                               exposingUuid, root);
}

void ControlSerializer::serializeNum(AxiomModel::NumControl *control, QDataStream &stream) {
    stream << (uint8_t) control->displayMode();
    stream << control->minValue();
    stream << control->maxValue();
    ValueSerializer::serializeNum(control->value(), stream);
}

std::unique_ptr<NumControl> ControlSerializer::deserializeNum(QDataStream &stream, uint32_t version, const QUuid &uuid,
                                                              const QUuid &parentUuid, QPoint pos, QSize size,
                                                              bool selected, QString name, bool showName,
                                                              QUuid exposerUuid, QUuid exposingUuid,
                                                              AxiomModel::ReferenceMapper *ref,
                                                              AxiomModel::ModelRoot *root) {
    uint8_t displayModeInt;
    stream >> displayModeInt;
    float minValue;
    stream >> minValue;
    float maxValue;
    stream >> maxValue;
    auto value = ValueSerializer::deserializeNum(stream, version);
    return NumControl::create(uuid, parentUuid, pos, size, selected, std::move(name), showName, exposerUuid,
                              exposingUuid, (NumControl::DisplayMode) displayModeInt, minValue, maxValue, value, root);
}

void ControlSerializer::serializePortal(AxiomModel::PortalControl *control, QDataStream &stream) {
    stream << (uint8_t) control->portalType();
    stream << (quint64) control->portalId();
}

std::unique_ptr<PortalControl> ControlSerializer::deserializePortal(
    QDataStream &stream, uint32_t version, const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
    bool selected, QString name, bool showName, QUuid exposerUuid, QUuid exposingUuid,
    AxiomModel::ConnectionWire::WireType wireType, AxiomModel::ReferenceMapper *ref, AxiomModel::ModelRoot *root) {
    uint8_t portalTypeInt;
    stream >> portalTypeInt;

    // unique portal IDs were added in 0.4.0, corresponding to schema version 5
    quint64 portalId;
    if (version >= 5) {
        stream >> portalId;
    } else {
        portalId = takeAt(dynamicCast<RootSurface *>(findChildren(root->nodeSurfaces(), QUuid())), 0)->takePortalId();
    }

    return PortalControl::create(uuid, parentUuid, pos, size, selected, std::move(name), showName, exposerUuid,
                                 exposingUuid, wireType, (PortalControl::PortalType) portalTypeInt, portalId, root);
}

void ControlSerializer::serializeGraph(GraphControl *control, QDataStream &stream) {
    auto savedState = control->state();
    stream << (bool) savedState;
    if (savedState) {
        stream << savedState->curveCount;
        stream << savedState->curveStartVals[0];
        for (uint8_t i = 0; i < savedState->curveCount; i++) {
            stream << savedState->curveStartVals[i + 1];
            stream << savedState->curveEndPositions[i];
            stream << savedState->curveTension[i];
            stream << savedState->curveStates[i];
        }
    }
}

std::unique_ptr<GraphControl> ControlSerializer::deserializeGraph(QDataStream &stream, uint32_t version,
                                                                  const QUuid &uuid, const QUuid &parentUuid,
                                                                  QPoint pos, QSize size, bool selected, QString name,
                                                                  bool showName, QUuid exposerUuid, QUuid exposingUuid,
                                                                  AxiomModel::ReferenceMapper *ref,
                                                                  AxiomModel::ModelRoot *root) {
    std::unique_ptr<GraphControlState> savedState;
    bool hasSavedState;
    stream >> hasSavedState;

    if (hasSavedState) {
        savedState = std::make_unique<GraphControlState>();
        stream >> savedState->curveCount;
        stream >> savedState->curveStartVals[0];
        for (uint8_t i = 0; i < savedState->curveCount; i++) {
            stream >> savedState->curveStartVals[i + 1];
            stream >> savedState->curveEndPositions[i];
            stream >> savedState->curveTension[i];
            stream >> savedState->curveStates[i];
        }
    }

    return GraphControl::create(uuid, parentUuid, pos, size, selected, std::move(name), showName, exposerUuid,
                                exposingUuid, std::move(savedState), root);
}

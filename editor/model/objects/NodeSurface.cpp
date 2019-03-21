#include "NodeSurface.h"

#include <QtCore/QMap>
#include <editor/model/serialize/ProjectSerializer.h>

#include "../IdentityReferenceMapper.h"
#include "../ModelRoot.h"
#include "../actions/CompositeAction.h"
#include "../serialize/ModelObjectSerializer.h"
#include "Connection.h"
#include "ControlSurface.h"
#include "GroupNode.h"
#include "GroupSurface.h"
#include "Node.h"
#include "RootSurface.h"
#include "editor/compiler/SurfaceMirBuilder.h"
#include "editor/compiler/interface/Runtime.h"

using namespace AxiomModel;

NodeSurface::NodeSurface(const QUuid &uuid, const QUuid &parentUuid, QPointF pan, float zoom,
                         AxiomModel::ModelRoot *root)
    : ModelObject(ModelType::NODE_SURFACE, uuid, parentUuid, root),
      _nodes(cacheSequence(findChildrenWatch(root->nodes(), uuid))),
      _connections(cacheSequence(findChildrenWatch(root->connections(), uuid))),
      _grid(AxiomCommon::boxWatchSequence(AxiomCommon::staticCastWatch<GridItem *>(_nodes.asRef())), true), _pan(pan),
      _zoom(zoom) {
    _nodes.events().itemAdded().connectTo(this, &NodeSurface::nodeAdded);

    _nodes.events().itemAdded().connectTo(this, &NodeSurface::setDirty);
    _nodes.events().itemRemoved().connectTo(this, &NodeSurface::setDirty);
    _connections.events().itemAdded().connectTo(this, &NodeSurface::setDirty);
    _connections.events().itemRemoved().connectTo(this, &NodeSurface::setDirty);
}

void NodeSurface::setPan(QPointF pan) {
    if (pan != _pan) {
        _pan = pan;
        panChanged(pan);
    }
}

void NodeSurface::setZoom(float zoom) {
    zoom = zoom < -0.5f ? -0.5f : zoom > 0.5f ? 0.5f : zoom;
    if (zoom != _zoom) {
        _zoom = zoom;
        zoomChanged(zoom);
    }
}

std::vector<ModelObject *> NodeSurface::getCopyItems() {
    // we want to copy:
    // all nodes and their children (but NOT nodes that aren't copyable!)
    // all connections that connect to controls in nodes that are selected

    auto copyNodes =
        AxiomCommon::filter(_nodes.sequence(), [](Node *node) { return node->isSelected() && node->isCopyable(); });
    auto poolSequence = AxiomCommon::collect(AxiomCommon::dynamicCast<ModelObject *>(pool()->sequence().sequence()));
    auto poolSequenceRef = AxiomCommon::refSequence(&poolSequence);
    auto copyChildren = AxiomCommon::flatten(AxiomCommon::map(
        copyNodes, [poolSequenceRef](Node *node) { return findDependents(poolSequenceRef, node->uuid()); }));

    // todo: can this only include controls that are on direct descendents of this surface?
    auto copyControls = AxiomCommon::dynamicCast<Control *>(copyChildren);
    QSet<QUuid> controlUuids;
    for (const auto &control : copyControls) {
        controlUuids.insert(control->uuid());
    }

    auto copyConnections = AxiomCommon::filter(_connections.sequence(), [controlUuids](Connection *connection) {
        return controlUuids.contains(connection->controlAUuid()) && controlUuids.contains(connection->controlBUuid());
    });

    return AxiomCommon::collect(AxiomCommon::flatten(std::array<AxiomCommon::BoxedSequence<ModelObject *>, 2>{
        AxiomCommon::boxSequence(copyChildren),
        AxiomCommon::boxSequence(AxiomCommon::staticCast<ModelObject *>(copyConnections))}));
}

void NodeSurface::groupSelectedNodes() {
    // The goal here is to group nodes while maintaining connections and the original behavior. Roughly, this means:
    //  - All selected nodes (except non-copyable ones) are moved into the group
    //  - Connections between nodes that are moved in are preserved in the group
    //  - Any controls that have connections to nodes _not_ in the group become exposed, and those connections are
    //    preserved on the top level.
    //  - For obvious reason, the above point also matters for controls that are already exposed - we should expose them
    //    inside the group, then again on this surface. It's also important to make sure exposing on this surface
    //    targets the original control.

    auto copyNodes = AxiomCommon::collect(
        AxiomCommon::filter(_nodes.sequence(), [](Node *node) { return node->isSelected() && node->isCopyable(); }));
    if (copyNodes.empty()) {
        return;
    }

    auto poolSequence = AxiomCommon::collect(AxiomCommon::dynamicCast<ModelObject *>(pool()->sequence().sequence()));
    auto poolSequenceRef = AxiomCommon::refSequence(&poolSequence);
    auto copyChildren =
        AxiomCommon::flatten(AxiomCommon::map(AxiomCommon::refSequence(&copyNodes), [poolSequenceRef](Node *node) {
            return findDependents(poolSequenceRef, node->uuid());
        }));
    auto copyControls = AxiomCommon::collect(
        AxiomCommon::filter(AxiomCommon::dynamicCast<Control *>(copyChildren),
                            [this](Control *control) { return control->surface()->node()->parentUuid() == uuid(); }));
    QSet<QUuid> controlUuids;
    for (const auto &control : copyControls) {
        controlUuids.insert(control->uuid());
    }

    // Determine up-front how to expose and connect things on this surface. It's important we do this here, since next
    // we're going to nuke all of the objects that we want to move into the new group, which will remove the
    // connections we care about here.
    QMap<QUuid, QUuid> exposedControlUuids; // maps current controls to new UUIDs
    for (const auto &control : copyControls) {
        QUuid exposedControlUuid = QUuid::createUuid();

        bool needsToBeExposed = false;
        if (!control->exposerUuid().isNull()) {
            needsToBeExposed = true;
        }

        for (const auto &connectedControl : control->connectedControls().sequence()) {
            if (controlUuids.contains(connectedControl)) continue;

            needsToBeExposed = true;

            // Make a new connection between the connected control and the new exposed one.
            // We can do this fine since the Connection constructor can't assume both controls exist.
            root()->pool().registerObj(
                Connection::create(QUuid::createUuid(), uuid(), connectedControl, exposedControlUuid, root()));
        }

        if (needsToBeExposed) {
            exposedControlUuids.insert(control->uuid(), exposedControlUuid);
        }
    }

    // Copy the items we want so we can delete them
    QByteArray serializeArray;
    QDataStream serializeStream(&serializeArray, QIODevice::WriteOnly);
    auto copyConnections = AxiomCommon::filter(_connections.sequence(), [controlUuids](Connection *connection) {
        return controlUuids.contains(connection->controlAUuid()) && controlUuids.contains(connection->controlBUuid());
    });
    ModelObjectSerializer::serializeChunk(
        serializeStream, uuid(),
        AxiomCommon::flatten(std::array<AxiomCommon::BoxedSequence<ModelObject *>, 2>{
            AxiomCommon::boxSequence(copyChildren),
            AxiomCommon::boxSequence(AxiomCommon::staticCast<ModelObject *>(copyConnections))}));

    for (const auto &copyNode : copyNodes) {
        copyNode->remove();
    }

    QUuid groupNodeUuid = QUuid::createUuid();
    QUuid controlsUuid = QUuid::createUuid();
    QUuid innerUuid = QUuid::createUuid();
    root()->pool().registerObj(GroupNode::create(groupNodeUuid, uuid(), QPoint(0, 0), QSize(3, 2), false, "",
                                                 controlsUuid, innerUuid, root()));
    root()->pool().registerObj(ControlSurface::create(controlsUuid, groupNodeUuid, root()));
    root()->pool().registerObj(GroupSurface::create(innerUuid, groupNodeUuid, QPoint(0, 0), 0, root()));

    // Copy the `copyChildren` and `copyConnections` into the new group.
    // Note: we don't need to worry about remapping UUIDs since they will still be unique
    IdentityReferenceMapper ref;
    QDataStream deserializeStream(&serializeArray, QIODevice::ReadOnly);
    ModelObjectSerializer::deserializeChunk(deserializeStream, ProjectSerializer::schemaVersion, root(), innerUuid,
                                            &ref, false);

    // Expose the controls that need to be exposed. We also handle exposing the secondary control if necessary here.
    auto end = exposedControlUuids.cend();
    for (auto it = exposedControlUuids.cbegin(); it != end; ++it) {
        auto sourceControlUuid = it.key();
        auto newControlUuid = it.value();

        auto sourceControl = find(root()->controls().sequence(), sourceControlUuid);
        auto prepareData = Control::buildControlPrepareAction(sourceControl->controlType(), controlsUuid, root());
        prepareData.preActions->forward(true);

        auto sourceControlExposerUuid = sourceControl->exposerUuid();
        auto newControl =
            Control::createExposed(sourceControl, newControlUuid, controlsUuid, prepareData.pos, prepareData.size);
        newControl->setExposerUuid(sourceControlExposerUuid);
        root()->pool().registerObj(std::move(newControl));
    }
}

void NodeSurface::forceCompile() {
    setDirty();
}

void NodeSurface::attachRuntime(MaximCompiler::Runtime *runtime) {
    _runtime = runtime;
    for (const auto &node : nodes().sequence()) {
        node->attachRuntime(runtime);
    }
}

void NodeSurface::updateRuntimePointers(MaximCompiler::Runtime *runtime, void *surfacePtr) {
    for (const auto &node : nodes().sequence()) {
        node->updateRuntimePointers(runtime, surfacePtr);
    }
}

void NodeSurface::build(MaximCompiler::Transaction *transaction) {
    MaximCompiler::SurfaceMirBuilder::build(transaction, this);
}

void NodeSurface::buildAll(MaximCompiler::Transaction *transaction) {
    for (const auto &node : nodes().sequence()) {
        node->buildAll(transaction);
    }

    build(transaction);
}

void NodeSurface::doRuntimeUpdate() {
    // flush the grid surfaces
    _grid.tryFlush();
    _wireGrid.tryFlush();

    for (const auto &node : nodes().sequence()) {
        if (auto controls = node->controls().value()) {
            for (const auto &control : (*controls)->controls().sequence()) {
                control->doRuntimeUpdate();
            }
        }
        node->doRuntimeUpdate();
    }
}

void NodeSurface::remove() {
    auto nodes = findChildren(root()->nodes().sequence(), uuid());
    while (!nodes.empty()) {
        (*nodes.begin())->remove();
    }
    auto connections = findChildren(root()->connections().sequence(), uuid());
    while (!connections.empty()) {
        (*connections.begin())->remove();
    }
    ModelObject::remove();
}

void NodeSurface::nodeAdded(AxiomModel::Node *node) {
    node->controls().then([this](ControlSurface *surface) {
        surface->controls().events().itemAdded().connectTo(this, &NodeSurface::setDirty);
        surface->controls().events().itemRemoved().connectTo(this, &NodeSurface::setDirty);

        surface->controls().events().itemAdded().connectTo(
            [this](Control *control) { control->exposerUuidChanged.connectTo(this, &NodeSurface::setDirty); });
    });

    if (_runtime) {
        node->attachRuntime(_runtime);
    }
}

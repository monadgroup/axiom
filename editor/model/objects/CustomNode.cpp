#include "CustomNode.h"

#include <iostream>

#include "ControlSurface.h"
#include "NumControl.h"
#include "MidiControl.h"
#include "ExtractControl.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../actions/CompositeAction.h"
#include "../actions/SetCodeAction.h"
#include "../actions/CreateControlAction.h"
#include "../actions/DeleteObjectAction.h"
#include "../../util.h"

using namespace AxiomModel;

CustomNode::CustomNode(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                       const QUuid &controlsUuid, QString code, bool panelOpen, float panelHeight,
                       AxiomModel::ModelRoot *root)
    : Node(NodeType::CUSTOM_NODE, uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid, root),
      _code(std::move(code)), _isPanelOpen(panelOpen), _panelHeight(panelHeight) {
}

std::unique_ptr<CustomNode> CustomNode::create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                               bool selected, QString name, const QUuid &controlsUuid, QString code,
                                               bool panelOpen, float panelHeight, AxiomModel::ModelRoot *root) {
    return std::make_unique<CustomNode>(uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid, code,
                                        panelOpen, panelHeight, root);
}

std::unique_ptr<CustomNode> CustomNode::deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid,
                                                    QPoint pos, QSize size, bool selected, QString name,
                                                    const QUuid &controlsUuid, ReferenceMapper *ref,
                                                    AxiomModel::ModelRoot *root) {
    QString code;
    stream >> code;
    bool isPanelOpen;
    stream >> isPanelOpen;
    float panelHeight;
    stream >> panelHeight;

    return create(uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid, code, isPanelOpen, panelHeight,
                  root);
}

void CustomNode::serialize(QDataStream &stream, const QUuid &parent, bool withContext) const {
    Node::serialize(stream, parent, withContext);
    stream << _code;
    stream << _isPanelOpen;
    stream << _panelHeight;
}

void CustomNode::setCode(const QString &code) {
    if (_code != code) {
        _code = code;
        codeChanged.trigger(code);
    }
}

void CustomNode::doSetCodeAction(QString beforeCode, QString afterCode) {
    std::vector<std::unique_ptr<Action>> actions;
    actions.push_back(SetCodeAction::create(uuid(), std::move(beforeCode), std::move(afterCode), root()));
    auto action = CompositeAction::create(std::move(actions), root());
    changeCodeAction = action.get();
    root()->history().append(std::move(action));
    changeCodeAction = nullptr;
}

void CustomNode::setPanelOpen(bool panelOpen) {
    if (_isPanelOpen != panelOpen) {
        _isPanelOpen = panelOpen;
        panelOpenChanged.trigger(panelOpen);
    }
}

void CustomNode::setPanelHeight(float panelHeight) {
    if (panelHeight < minPanelHeight) panelHeight = minPanelHeight;
    if (_panelHeight != panelHeight) {
        beforePanelHeightChanged.trigger(panelHeight);
        _panelHeight = panelHeight;
        panelHeightChanged.trigger(panelHeight);
    }
}

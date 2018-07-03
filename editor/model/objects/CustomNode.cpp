#include "CustomNode.h"

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
#include "editor/compiler/interface/Runtime.h"

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

        if (_runtime) {
            buildCode();
        }
    }
}

void CustomNode::doSetCodeAction(QString beforeCode, QString afterCode) {
    auto action = CompositeAction::create({}, root());
    auto setCodeAction = SetCodeAction::create(uuid(), std::move(beforeCode), std::move(afterCode), root());
    setCodeAction->forward(true);
    action->actions().push_back(std::move(setCodeAction));
    updateControls(action.get());
    root()->history().append(std::move(action), false);
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

uint64_t CustomNode::getRuntimeId(MaximCompiler::Runtime &runtime) {
    if (runtimeId) {
        return runtimeId;
    } else {
        return runtime.nextId();
    }
}

void CustomNode::attachRuntime(MaximCompiler::Runtime *runtime) {
    std::cout << "Attached runtime" << std::endl;
    _runtime = runtime;
    if (runtime) {
        buildCode();
    }
}

std::optional<MaximCompiler::Block> CustomNode::compiledBlock() const {
    if (_compiledBlock) {
        return std::optional(_compiledBlock->clone());
    } else {
        return std::nullopt;
    }
}

void CustomNode::buildCode() {
    auto blockId = getRuntimeId(*_runtime);
    auto compileResult = MaximCompiler::Block::compile(blockId, name(), code());

    if (MaximCompiler::Error *err = std::get_if<MaximCompiler::Error>(&compileResult)) {
        auto errorDescription = err->getDescription();
        auto errorRange = err->getRange();
        std::cerr << "Error at " << errorRange.front.line << ":" << errorRange.front.column << " -> "
                  << errorRange.back.line << ":" << errorRange.back.column << " : " << errorDescription.toStdString()
                  << std::endl;
        codeCompileError.trigger(errorDescription, errorRange);
    } else {
        _compiledBlock = std::optional<MaximCompiler::Block>(std::move(std::get<MaximCompiler::Block>(compileResult)));
    }
}

void CustomNode::updateControls(CompositeAction *actionGroup) {
    if (!_compiledBlock || !controls().value()) return;

    QSet<QUuid> retainedControls;
    auto controlList = collect((*controls().value())->controls());
    for (size_t controlIndex = 0; controlIndex < _compiledBlock->controlCount(); controlIndex++) {
        auto compiledControl = _compiledBlock->getControl(controlIndex);
        auto compiledModelType = MaximCompiler::toModelType(compiledControl.getType());
        auto compiledName = compiledControl.getName();

        // find a control that matches name and type
        // todo: after all name matches have been claimed, assign to an unnamed one (allows renaming in code to rename
        // the actual control)
        auto foundControl = false;
        for (const auto &candidateControl : controlList) {
            if (candidateControl->name() == compiledName && candidateControl->controlType() == compiledModelType) {
                // todo: assign control metadata - we will need the index and read/write flags

                retainedControls.insert(candidateControl->uuid());
                foundControl = true;
                break;
            }
        }

        // no candidate found, create a new one
        if (!foundControl) {
            auto createAction = CreateControlAction::create((*controls().value())->uuid(), compiledModelType,
                                                            compiledName, root());
            createAction->forward(true);
            actionGroup->actions().push_back(std::move(createAction));

            // todo: assign control metadata
        }
    }

    // remove any controls that weren't retained
    for (const auto &existingControl : controlList) {
        if (retainedControls.contains(existingControl->uuid())) continue;

        auto deleteAction = DeleteObjectAction::create(existingControl->uuid(), root());
        deleteAction->forward(true);
        actionGroup->actions().push_back(std::move(deleteAction));
    }
}

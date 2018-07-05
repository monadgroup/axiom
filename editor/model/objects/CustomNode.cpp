#include "CustomNode.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../actions/CompositeAction.h"
#include "../actions/CreateControlAction.h"
#include "../actions/DeleteObjectAction.h"
#include "../actions/SetCodeAction.h"
#include "ControlSurface.h"
#include "ExtractControl.h"
#include "MidiControl.h"
#include "NumControl.h"
#include "editor/compiler/interface/Runtime.h"

using namespace AxiomModel;

CustomNode::CustomNode(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                       const QUuid &controlsUuid, QString code, bool panelOpen, float panelHeight,
                       AxiomModel::ModelRoot *root)
    : Node(NodeType::CUSTOM_NODE, uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid, root),
      _code(std::move(code)), _isPanelOpen(panelOpen), _panelHeight(panelHeight) {}

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

        if (runtimeId) {
            buildCode();
        }
    }
}

void CustomNode::doSetCodeAction(QString beforeCode, QString afterCode) {
    /*auto action = CompositeAction::create({}, root());
    auto setCodeAction = SetCodeAction::create(uuid(), std::move(beforeCode), std::move(afterCode), root());
    setCodeAction->forward(true);
    action->actions().push_back(std::move(setCodeAction));
    //updateControls(action.get());
    root()->history().append(std::move(action), false);*/

    auto setCodeAction = SetCodeAction::create(uuid(), std::move(beforeCode), std::move(afterCode), root());
    root()->history().append(std::move(setCodeAction));
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

void CustomNode::attachRuntime(MaximCompiler::Runtime *runtime, MaximCompiler::Transaction *transaction) {
    if (runtime) {
        runtimeId = runtime->nextId();
        buildCode();
    } else {
        runtimeId = 0;
    }

    if (transaction) {
        build(transaction);
    }
}

std::optional<MaximCompiler::Block> CustomNode::compiledBlock() const {
    if (_compiledBlock) {
        return std::optional(_compiledBlock->clone());
    } else {
        return std::nullopt;
    }
}

void CustomNode::build(MaximCompiler::Transaction *transaction) {
    if (!_compiledBlock) return;
    transaction->buildBlock(_compiledBlock->clone());
}

void CustomNode::updateControls() {
    if (!_compiledBlock || !controls().value()) return;

    QSet<QUuid> retainedControls;
    auto controlList = collect((*controls().value())->controls());
    for (size_t controlIndex = 0; controlIndex < _compiledBlock->controlCount(); controlIndex++) {
        auto compiledControl = _compiledBlock->getControl(controlIndex);
        auto compiledModelType = MaximCompiler::toModelType(compiledControl.getType());
        auto compiledName = compiledControl.getName();
        ControlCompileMeta compileMeta(controlIndex, compiledControl.getIsWritten(), compiledControl.getIsRead());

        // find a control that matches name and type
        // todo: after all name matches have been claimed, assign to an unnamed one
        // (allows renaming in code to rename the actual control)
        auto foundControl = false;
        for (const auto &candidateControl : controlList) {
            if (candidateControl->name() == compiledName && candidateControl->controlType() == compiledModelType) {
                std::cout << "Found candidate control for " << compiledName.toStdString() << std::endl;
                candidateControl->setCompileMeta(compileMeta);
                retainedControls.insert(candidateControl->uuid());
                foundControl = true;
                break;
            }
        }

        // no candidate found, create a new one
        if (!foundControl) {
            std::cout << "Creating new control for " << compiledName.toStdString() << std::endl;
            auto createAction =
                CreateControlAction::create((*controls().value())->uuid(), compiledModelType, compiledName, root());
            createAction->forward(true, nullptr);

            auto newControl = find(root()->controls(), createAction->getUuid());
            newControl->setCompileMeta(compileMeta);

            /*if (actionGroup) {
                actionGroup->actions().push_back(std::move(createAction));
            }*/
        }
    }

    // remove any controls that weren't retained
    for (const auto &existingControl : controlList) {
        if (retainedControls.contains(existingControl->uuid())) continue;

        std::cout << "Deleting old control " << existingControl->name().toStdString() << std::endl;
        auto deleteAction = DeleteObjectAction::create(existingControl->uuid(), root());
        deleteAction->forward(true, nullptr);

        /*if (actionGroup) {
            actionGroup->actions().push_back(std::move(deleteAction));
        }*/
    }
}

void CustomNode::buildCode() {
    auto compileResult = MaximCompiler::Block::compile(getRuntimeId(), name(), code());

    if (MaximCompiler::Error *err = std::get_if<MaximCompiler::Error>(&compileResult)) {
        auto errorDescription = err->getDescription();
        auto errorRange = err->getRange();
        std::cerr << "Error at " << errorRange.front.line << ":" << errorRange.front.column << " -> "
                  << errorRange.back.line << ":" << errorRange.back.column << " : " << errorDescription.toStdString()
                  << std::endl;
        codeCompileError.trigger(errorDescription, errorRange);
    } else {
        _compiledBlock = std::optional<MaximCompiler::Block>(std::move(std::get<MaximCompiler::Block>(compileResult)));
        codeCompileSuccess.trigger();
    }
}

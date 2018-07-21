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
#include "NodeSurface.h"
#include "NumControl.h"
#include "editor/compiler/interface/Runtime.h"

using namespace AxiomModel;

CustomNode::CustomNode(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                       const QUuid &controlsUuid, QString code, bool panelOpen, float panelHeight,
                       AxiomModel::ModelRoot *root)
    : Node(NodeType::CUSTOM_NODE, uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid, root),
      _code(std::move(code)), _isPanelOpen(panelOpen), _panelHeight(panelHeight) {
    controls().then([this](ControlSurface *controls) {
        controls->controls().itemAdded.connect(this, &CustomNode::surfaceControlAdded);
    });
}

std::unique_ptr<CustomNode> CustomNode::create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                               bool selected, QString name, const QUuid &controlsUuid, QString code,
                                               bool panelOpen, float panelHeight, AxiomModel::ModelRoot *root) {
    return std::make_unique<CustomNode>(uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid, code,
                                        panelOpen, panelHeight, root);
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
    std::vector<QUuid> compileItems;
    auto setCodeAction = SetCodeAction::create(uuid(), std::move(beforeCode), std::move(afterCode), {}, root());
    setCodeAction->forward(true, compileItems);
    updateControls(setCodeAction.get());
    root()->history().append(std::move(setCodeAction), false);
    root()->applyCompile(compileItems);
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
        updateControls(nullptr);
    }
}

void CustomNode::updateRuntimePointers(MaximCompiler::Runtime *runtime, void *surfacePtr) {
    Node::updateRuntimePointers(runtime, surfacePtr);

    auto nodePtr = runtime->getNodePtr(surface()->getRuntimeId(), surfacePtr, compileMeta()->mirIndex);
    auto blockPtr = runtime->getBlockPtr(nodePtr);
    auto runtimeId = getRuntimeId();

    controls().then([blockPtr, runtime, runtimeId](ControlSurface *controlSurface) {
        for (const auto &control : controlSurface->controls()) {
            control->setRuntimePointers(runtime->getControlPtrs(runtimeId, blockPtr, control->compileMeta()->index));
        }
    });
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

struct NewControl {
    Control::ControlType type;
    QString name;
};

void CustomNode::updateControls(SetCodeAction *action) {
    if (!_compiledBlock || !controls().value()) return;

    auto compiledControlCount = _compiledBlock->controlCount();

    std::vector<NewControl> newControls;

    QSet<QUuid> retainedControls;
    auto controlList = collect((*controls().value())->controls());
    for (size_t controlIndex = 0; controlIndex < compiledControlCount; controlIndex++) {
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
                candidateControl->setCompileMeta(compileMeta);
                retainedControls.insert(candidateControl->uuid());
                foundControl = true;
                break;
            }
        }

        // no candidate found, queue a new one
        if (!foundControl) {
            newControls.push_back(NewControl{compiledModelType, std::move(compiledName)});
        }
    }

    // remove any controls that weren't retained
    for (const auto &existingControl : controlList) {
        if (retainedControls.contains(existingControl->uuid())) continue;

        auto deleteAction = DeleteObjectAction::create(existingControl->uuid(), root());
        std::vector<QUuid> dummy;
        deleteAction->forward(true, dummy);

        assert(action);
        action->controlActions().push_back(std::move(deleteAction));
    }

    // add any new controls
    // note: the order we do this is important: we want to add controls after removing old ones so the control grid
    // can have the space cleared
    for (const auto &newControl : newControls) {
        auto createAction = CreateControlAction::create((*controls().value())->uuid(), newControl.type,
                                                        std::move(newControl.name), root());
        std::vector<QUuid> dummy;
        createAction->forward(true, dummy);

        assert(action);
        action->controlActions().push_back(std::move(createAction));
    }
}

void CustomNode::surfaceControlAdded(AxiomModel::Control *control) {
    if (!_compiledBlock) return;
    assert(!control->compileMeta());

    // find a matching control that we can claim
    auto compiledControlCount = _compiledBlock->controlCount();
    for (size_t controlIndex = 0; controlIndex < compiledControlCount; controlIndex++) {
        auto compiledControl = _compiledBlock->getControl(controlIndex);
        auto compiledModelType = MaximCompiler::toModelType(compiledControl.getType());

        // looks like we can claim it!
        if (control->controlType() == compiledModelType && control->name() == compiledControl.getName()) {
            ControlCompileMeta compileMeta(controlIndex, compiledControl.getIsWritten(), compiledControl.getIsRead());
            control->setCompileMeta(compileMeta);
            break;
        }
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

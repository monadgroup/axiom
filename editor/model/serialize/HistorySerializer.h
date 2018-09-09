#pragma once

#include <QtCore/QDataStream>
#include <memory>

#include "../HistoryList.h"

namespace AxiomModel {

    class Action;
    class CompositeAction;
    class DeleteObjectAction;
    class CreateCustomNodeAction;
    class CreateGroupNodeAction;
    class CreatePortalNodeAction;
    class CreateConnectionAction;
    class GridItemMoveAction;
    class GridItemSizeAction;
    class RenameControlAction;
    class RenameNodeAction;
    class SetCodeAction;
    class CreateControlAction;
    class SetNumModeAction;
    class SetNumValueAction;
    class SetShowNameAction;
    class ExposeControlAction;
    class PasteBufferAction;
    class UnexposeControlAction;
    class AddGraphPointAction;
    class DeleteGraphPointAction;
    class MoveGraphPointAction;
    class SetGraphTagAction;
    class SetGraphTensionAction;

    namespace HistorySerializer {
        void serialize(const HistoryList &history, QDataStream &stream);

        HistoryList deserialize(QDataStream &stream, uint32_t version, ModelRoot *root,
                                HistoryList::CompileApplyer applyer);

        void serializeAction(Action *action, QDataStream &stream);

        std::unique_ptr<Action> deserializeAction(QDataStream &stream, uint32_t version, ModelRoot *root);

        void serializeCompositeAction(CompositeAction *action, QDataStream &stream);

        std::unique_ptr<Action> deserializeCompositeAction(QDataStream &stream, uint32_t version, ModelRoot *root);

        void serializeDeleteObjectAction(DeleteObjectAction *action, QDataStream &stream);

        std::unique_ptr<DeleteObjectAction> deserializeDeleteObjectAction(QDataStream &stream, uint32_t version,
                                                                          ModelRoot *root);

        void serializeCreateCustomNodeAction(CreateCustomNodeAction *action, QDataStream &stream);

        std::unique_ptr<CreateCustomNodeAction> deserializeCreateCustomNodeAction(QDataStream &stream, uint32_t version,
                                                                                  ModelRoot *root);

        void serializeCreateGroupNodeAction(CreateGroupNodeAction *action, QDataStream &stream);

        std::unique_ptr<CreateGroupNodeAction> deserializeCreateGroupNodeAction(QDataStream &stream, uint32_t version,
                                                                                ModelRoot *root);

        void serializeCreatePortalNodeAction(CreatePortalNodeAction *action, QDataStream &stream);

        std::unique_ptr<CreatePortalNodeAction> deserializeCreatePortalNodeAction(QDataStream &stream, uint32_t version,
                                                                                  ModelRoot *root);

        void serializeCreateConnectionAction(CreateConnectionAction *action, QDataStream &stream);

        std::unique_ptr<CreateConnectionAction> deserializeCreateConnectionAction(QDataStream &stream, uint32_t version,
                                                                                  ModelRoot *root);

        void serializeGridItemMoveAction(GridItemMoveAction *action, QDataStream &stream);

        std::unique_ptr<GridItemMoveAction> deserializeGridItemMoveAction(QDataStream &stream, uint32_t version,
                                                                          ModelRoot *root);

        void serializeGridItemSizeAction(GridItemSizeAction *action, QDataStream &stream);

        std::unique_ptr<GridItemSizeAction> deserializeGridItemSizeAction(QDataStream &stream, uint32_t version,
                                                                          ModelRoot *root);

        void serializeRenameControlAction(RenameControlAction *action, QDataStream &stream);

        std::unique_ptr<RenameControlAction> deserializeRenameControlAction(QDataStream &stream, uint32_t version,
                                                                            ModelRoot *root);

        void serializeRenameNodeAction(RenameNodeAction *action, QDataStream &stream);

        std::unique_ptr<RenameNodeAction> deserializeRenameNodeAction(QDataStream &stream, uint32_t version,
                                                                      ModelRoot *root);

        void serializeSetCodeAction(SetCodeAction *action, QDataStream &stream);

        std::unique_ptr<SetCodeAction> deserializeSetCodeAction(QDataStream &stream, uint32_t version, ModelRoot *root);

        void serializeCreateControlAction(CreateControlAction *action, QDataStream &stream);

        std::unique_ptr<CreateControlAction> deserializeCreateControlAction(QDataStream &stream, uint32_t version,
                                                                            ModelRoot *root);

        void serializeSetNumModeAction(SetNumModeAction *action, QDataStream &stream);

        std::unique_ptr<SetNumModeAction> deserializeSetNumModeAction(QDataStream &stream, uint32_t version,
                                                                      ModelRoot *root);

        void serializeSetNumValueAction(SetNumValueAction *action, QDataStream &stream);

        std::unique_ptr<SetNumValueAction> deserializeSetNumValueAction(QDataStream &stream, uint32_t version,
                                                                        ModelRoot *root);

        void serializeSetShowNameAction(SetShowNameAction *action, QDataStream &stream);

        std::unique_ptr<SetShowNameAction> deserializeSetShowNameAction(QDataStream &stream, uint32_t version,
                                                                        ModelRoot *root);

        void serializeExposeControlAction(ExposeControlAction *action, QDataStream &stream);

        std::unique_ptr<ExposeControlAction> deserializeExposeControlAction(QDataStream &stream, uint32_t version,
                                                                            ModelRoot *root);

        void serializePasteBufferAction(PasteBufferAction *action, QDataStream &stream);

        std::unique_ptr<PasteBufferAction> deserializePasteBufferAction(QDataStream &stream, uint32_t version,
                                                                        ModelRoot *root);

        void serializeUnexposeControlAction(UnexposeControlAction *action, QDataStream &stream);

        std::unique_ptr<UnexposeControlAction> deserializeUnexposeControlAction(QDataStream &stream, uint32_t version,
                                                                                ModelRoot *root);

        void serializeAddGraphPointAction(AddGraphPointAction *action, QDataStream &stream);

        std::unique_ptr<AddGraphPointAction> deserializeAddGraphPointAction(QDataStream &stream, uint32_t version,
                                                                            ModelRoot *root);

        void serializeDeleteGraphPointAction(DeleteGraphPointAction *action, QDataStream &stream);

        std::unique_ptr<DeleteGraphPointAction> deserializeDeleteGraphPointAction(QDataStream &stream, uint32_t version,
                                                                                  ModelRoot *root);

        void serializeMoveGraphPointAction(MoveGraphPointAction *action, QDataStream &stream);

        std::unique_ptr<MoveGraphPointAction> deserializeMoveGraphPointAction(QDataStream &stream, uint32_t version,
                                                                              ModelRoot *root);

        void serializeSetGraphTagAction(SetGraphTagAction *action, QDataStream &stream);

        std::unique_ptr<SetGraphTagAction> deserializeSetGraphTagAction(QDataStream &stream, uint32_t version,
                                                                        ModelRoot *root);

        void serializeSetGraphTensionAction(SetGraphTensionAction *action, QDataStream &stream);

        std::unique_ptr<SetGraphTensionAction> deserializeSetGraphTensionAction(QDataStream &stream, uint32_t version,
                                                                                ModelRoot *root);
    }
}

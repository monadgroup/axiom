#pragma once

#include <QtCore/QUuid>

#include "DeleteObjectAction.h"

namespace AxiomModel {

    class UnexposeControlAction : public Action {
    public:
        UnexposeControlAction(const QUuid &controlUuid, std::unique_ptr<DeleteObjectAction> deleteExposerAction,
                              ModelRoot *root);

        static std::unique_ptr<UnexposeControlAction>
            create(const QUuid &controlUuid, std::unique_ptr<DeleteObjectAction> deleteExposerAction, ModelRoot *root);

        static std::unique_ptr<UnexposeControlAction> create(const QUuid &controlUuid, ModelRoot *root);

        void forward(bool first) override;

        void backward() override;

        const QUuid &controlUuid() const { return _controlUuid; }

        DeleteObjectAction *deleteExposerAction() const { return _deleteExposerAction.get(); }

    private:
        QUuid _controlUuid;
        std::unique_ptr<DeleteObjectAction> _deleteExposerAction;
    };
}

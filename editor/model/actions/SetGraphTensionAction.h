#pragma once

#include <QtCore/QUuid>

#include "Action.h"

namespace AxiomModel {

    class SetGraphTensionAction : public Action {
    public:
        SetGraphTensionAction(const QUuid &controlUuid, uint8_t index, float oldTension, float newTension,
                              ModelRoot *root);

        static std::unique_ptr<SetGraphTensionAction> create(const QUuid &controlUuid, uint8_t index, float oldTension,
                                                             float newTension, ModelRoot *root);

        void forward(bool first, std::vector<QUuid> &compileItems) override;

        void backward(std::vector<QUuid> &compileItems) override;

        const QUuid &controlUuid() const { return _controlUuid; }

        uint8_t index() const { return _index; }

        float oldTension() const { return _oldTension; }

        float newTension() const { return _newTension; }

    private:
        QUuid _controlUuid;
        uint8_t _index;
        float _oldTension;
        float _newTension;
    };
}

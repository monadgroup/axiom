#pragma once

#include <QtCore/QUuid>

#include "Action.h"

namespace AxiomModel {

    class SetGraphTensionAction : public Action {
    public:
        SetGraphTensionAction(const QUuid &controlUuid, uint8_t index, double oldTension, double newTension,
                              ModelRoot *root);

        static std::unique_ptr<SetGraphTensionAction> create(const QUuid &controlUuid, uint8_t index, double oldTension,
                                                             double newTension, ModelRoot *root);

        void forward(bool first) override;

        void backward() override;

        const QUuid &controlUuid() const { return _controlUuid; }

        uint8_t index() const { return _index; }

        float oldTension() const { return _oldTension; }

        float newTension() const { return _newTension; }

    private:
        QUuid _controlUuid;
        uint8_t _index;
        double _oldTension;
        double _newTension;
    };
}

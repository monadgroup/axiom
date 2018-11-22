#pragma once

#include <QtCore/QUuid>

#include "Action.h"

namespace AxiomModel {

    class MoveGraphPointAction : public Action {
    public:
        MoveGraphPointAction(const QUuid &controlUuid, uint8_t index, double oldTime, double oldValue, double newTime,
                             double newValue, ModelRoot *root);

        static std::unique_ptr<MoveGraphPointAction> create(const QUuid &controlUuid, uint8_t index, double oldTime,
                                                            double oldValue, double newTime, double newValue,
                                                            ModelRoot *root);

        void forward(bool first) override;

        void backward() override;

        const QUuid &controlUuid() const { return _controlUuid; }

        uint8_t index() const { return _index; }

        double oldTime() const { return _oldTime; }

        double oldValue() const { return _oldValue; }

        double newTime() const { return _newTime; }

        double newValue() const { return _newValue; }

    private:
        QUuid _controlUuid;
        uint8_t _index;
        double _oldTime;
        double _oldValue;
        double _newTime;
        double _newValue;
    };
}

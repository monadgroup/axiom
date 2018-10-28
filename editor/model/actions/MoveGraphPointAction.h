#pragma once

#include <QtCore/QUuid>

#include "Action.h"

namespace AxiomModel {

    class MoveGraphPointAction : public Action {
    public:
        MoveGraphPointAction(const QUuid &controlUuid, uint8_t index, float oldTime, float oldValue, float newTime,
                             float newValue, ModelRoot *root);

        static std::unique_ptr<MoveGraphPointAction> create(const QUuid &controlUuid, uint8_t index, float oldTime,
                                                            float oldValue, float newTime, float newValue,
                                                            ModelRoot *root);

        void forward(bool first) override;

        void backward() override;

        const QUuid &controlUuid() const { return _controlUuid; }

        uint8_t index() const { return _index; }

        float oldTime() const { return _oldTime; }

        float oldValue() const { return _oldValue; }

        float newTime() const { return _newTime; }

        float newValue() const { return _newValue; }

    private:
        QUuid _controlUuid;
        uint8_t _index;
        float _oldTime;
        float _oldValue;
        float _newTime;
        float _newValue;
    };
}

#pragma once

#include <QtCore/QUuid>

#include "Action.h"

namespace AxiomModel {

    class DeleteGraphPointAction : public Action {
    public:
        DeleteGraphPointAction(const QUuid &controlUuid, uint8_t index, float time, float val, float tension,
                               uint8_t state, ModelRoot *root);

        static std::unique_ptr<DeleteGraphPointAction> create(const QUuid &controlUuid, uint8_t index, float time,
                                                              float val, float tension, uint8_t state, ModelRoot *root);

        void forward(bool first) override;

        void backward() override;

        const QUuid &controlUuid() const { return _controlUuid; }

        uint8_t index() const { return _index; }

        float time() const { return _time; }

        float val() const { return _val; }

        float tension() const { return _tension; }

        uint8_t state() const { return _state; }

    private:
        QUuid _controlUuid;
        uint8_t _index;
        float _time;
        float _val;
        float _tension;
        uint8_t _state;
    };
}

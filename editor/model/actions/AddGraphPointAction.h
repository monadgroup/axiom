#pragma once

#include <QtCore/QUuid>

#include "Action.h"

namespace AxiomModel {

    class AddGraphPointAction : public Action {
    public:
        AddGraphPointAction(const QUuid &controlUuid, uint8_t index, float time, float val, ModelRoot *root);

        static std::unique_ptr<AddGraphPointAction> create(const QUuid &controlUuid, uint8_t index, float time,
                                                           float val, ModelRoot *root);

        void forward(bool first) override;

        void backward() override;

        const QUuid &controlUuid() const { return _controlUuid; }

        uint8_t index() const { return _index; }

        float time() const { return _time; }

        float val() const { return _val; }

    private:
        QUuid _controlUuid;
        uint8_t _index;
        float _time;
        float _val;
    };
}

#pragma once

#include <QtCore/QUuid>

#include "Action.h"

namespace AxiomModel {

    class AddGraphPointAction : public Action {
    public:
        AddGraphPointAction(const QUuid &controlUuid, uint8_t index, float time, float val, float tension,
                            ModelRoot *root);

        static std::unique_ptr<AddGraphPointAction> create(const QUuid &controlUuid, uint8_t index, float time,
                                                           float val, float tension, ModelRoot *root);

        void forward(bool first, std::vector<QUuid> &compileItems) override;

        void backward(std::vector<QUuid> &compileItems) override;

        const QUuid &controlUuid() const { return _controlUuid; }

        uint8_t index() const { return _index; }

        float time() const { return _time; }

        float val() const { return _val; }

        float tension() const { return _tension; }

    private:
        QUuid _controlUuid;
        uint8_t _index;
        float _time;
        float _val;
        float _tension;
    };
}

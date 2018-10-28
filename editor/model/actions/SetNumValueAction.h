#pragma once

#include <QtCore/QUuid>

#include "../Value.h"
#include "Action.h"

namespace AxiomModel {

    class SetNumValueAction : public Action {
    public:
        SetNumValueAction(const QUuid &uuid, NumValue beforeVal, NumValue afterVal, ModelRoot *root);

        static std::unique_ptr<SetNumValueAction> create(const QUuid &uuid, NumValue beforeVal, NumValue afterVal,
                                                         ModelRoot *root);

        void forward(bool first) override;

        void backward() override;

        const QUuid &uuid() const { return _uuid; }

        const NumValue &beforeVal() const { return _beforeVal; }

        const NumValue &afterVal() const { return _afterVal; }

    private:
        QUuid _uuid;
        NumValue _beforeVal;
        NumValue _afterVal;
    };
}

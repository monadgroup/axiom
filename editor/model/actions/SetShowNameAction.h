#pragma once

#include <QtCore/QUuid>

#include "Action.h"

namespace AxiomModel {

    class SetShowNameAction : public Action {
    public:
        SetShowNameAction(const QUuid &uuid, bool beforeVal, bool afterVal, ModelRoot *root);

        static std::unique_ptr<SetShowNameAction> create(const QUuid &uuid, bool beforeVal, bool afterVal,
                                                         ModelRoot *root);

        void forward(bool first) override;

        void backward() override;

        const QUuid &uuid() const { return _uuid; }

        const bool &beforeVal() const { return _beforeVal; }

        const bool &afterVal() const { return _afterVal; }

    private:
        QUuid _uuid;
        bool _beforeVal;
        bool _afterVal;
    };
}

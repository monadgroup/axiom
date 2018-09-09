#pragma once

#include <QtCore/QUuid>

#include "Action.h"

namespace AxiomModel {

    class SetGraphTagAction : public Action {
    public:
        SetGraphTagAction(const QUuid &controlUuid, uint8_t index, uint8_t oldTag, uint8_t newTag, ModelRoot *root);

        static std::unique_ptr<SetGraphTagAction> create(const QUuid &controlUuid, uint8_t index, uint8_t oldTag,
                                                         uint8_t newTag, ModelRoot *root);

        void forward(bool first, std::vector<QUuid> &compileItems) override;

        void backward(std::vector<QUuid> &compileItems) override;

        const QUuid &controlUuid() const { return _controlUuid; }

        uint8_t index() const { return _index; }

        uint8_t oldTag() const { return _oldTag; }

        uint8_t newTag() const { return _newTag; }

    private:
        QUuid _controlUuid;
        uint8_t _index;
        uint8_t _oldTag;
        uint8_t _newTag;
    };
}

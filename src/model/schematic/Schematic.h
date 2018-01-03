#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QPointF>

#include "../GridSurface.h"
#include "../node/Node.h"

namespace AxiomModel {

    class Schematic : public GridSurface {
    Q_OBJECT

    public:
        virtual QString name() = 0;

        QPointF pan() const { return m_pan; }

        virtual void serialize(QDataStream &stream) const;

        virtual void deserialize(QDataStream &stream);

    public slots:

        void setPan(QPointF pan);

    signals:

        void panChanged(QPointF newPan);

    private:
        QPointF m_pan;
    };

}

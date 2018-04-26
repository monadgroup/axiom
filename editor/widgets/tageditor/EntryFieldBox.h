#pragma once

#include <QtWidgets/QLabel>

namespace AxiomGui {

    class EntryFieldBox : public QLabel {
        Q_OBJECT

    public:
        EntryFieldBox(const QString &name, QWidget *parent = nullptr);
    };

}

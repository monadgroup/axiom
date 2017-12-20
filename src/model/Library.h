#pragma once
#include <memory>
#include <QtCore/QObject>
#include <QtCore/QVector>
#include <QtCore/QMap>
#include <QtCore/QString>

namespace AxiomModel {

    class LibraryEntry;

    class Library {
        Q_OBJECT

    public:
        QVector<std::unique_ptr<LibraryEntry>> entries;
        QMap<QString, QVector<LibraryEntry*>> index;
    };

}

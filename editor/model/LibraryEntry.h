#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <set>

#include "schematic/LibrarySchematic.h"

namespace AxiomModel {

    class Library;

    class LibraryEntry : public QObject {
    Q_OBJECT

    public:
        LibraryEntry(QString name, std::set<QString> tags);

        QString name() const { return _name; }

        const std::set<QString> &tags() const { return _tags; }

        const LibrarySchematic &schematic() const { return _schematic; }

        void serialize(QDataStream &stream) const;

        void deserialize(QDataStream &stream);

    public slots:

        void setName(const QString &name);

        void addTag(const QString &tag);

        void removeTag(const QString &tag);

    signals:

        void nameChanged(const QString &newName);

        void tagAdded(const QString &tag);

        void tagRemoved(const QString &tag);

    private:

        QString _name;
        std::set<QString> _tags;
        LibrarySchematic _schematic;
    };

}

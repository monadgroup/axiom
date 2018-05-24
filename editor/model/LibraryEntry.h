#pragma once

#include <QtCore/QString>
#include <QtCore/QDateTime>

#include "common/Event.h"
#include "ModelRoot.h"

namespace AxiomModel {

    class Library;

    class LibraryEntry : public AxiomCommon::Hookable {
    public:
        AxiomCommon::Event<const QString &> nameChanged;
        AxiomCommon::Event<const QString &> tagAdded;
        AxiomCommon::Event<const QString &> tagRemoved;
        AxiomCommon::Event<> removed;
        AxiomCommon::Event<> cleanup;

        LibraryEntry(QString name, const QUuid &baseUuid, const QUuid &modificationUuid, const QDateTime &modificationDateTime, std::set<QString> tags, std::unique_ptr<ModelRoot> root);

        static std::unique_ptr<LibraryEntry> create(QString name, const QUuid &baseUuid, const QUuid &modificationUuid, const QDateTime &modificationDateTime, std::set<QString> tags, std::unique_ptr<ModelRoot> root);

        static std::unique_ptr<LibraryEntry> create(QString name, std::set<QString> tags);

        static std::unique_ptr<LibraryEntry> deserialize(QDataStream &stream);

        void serialize(QDataStream &stream);

        const QString &name() const { return _name; }

        void setName(const QString &newName);

        const QUuid &baseUuid() const { return _baseUuid; }

        const QUuid &modificationUuid() const { return _modificationUuid; }

        const QDateTime &modificationDateTime() const { return _modificationDateTime; }

        const std::set<QString> &tags() const { return _tags; }

        void addTag(const QString &tag);

        void removeTag(const QString &tag);

        ModelRoot *root() const { return _root.get(); }

        void modified();

        void remove();

    private:
        QString _name;
        QUuid _baseUuid;
        QUuid _modificationUuid;
        QDateTime _modificationDateTime;
        std::set<QString> _tags;
        std::unique_ptr<ModelRoot> _root;
    };

}

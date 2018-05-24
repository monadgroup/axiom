#pragma once

#include <QtCore/QString>
#include <QtCore/QUuid>
#include <map>
#include <vector>

#include "common/Event.h"

namespace AxiomModel {

    class LibraryEntry;

    class Library : public AxiomCommon::Hookable {
    public:
        AxiomCommon::Event<LibraryEntry *> entryAdded;
        AxiomCommon::Event<const QString &> activeTagChanged;
        AxiomCommon::Event<const QString &> tagAdded;
        AxiomCommon::Event<const QString &> tagRemoved;

        Library();

        explicit Library(QDataStream &stream);

        ~Library();

        void serialize(QDataStream &stream);

        const QString &activeTag() const { return _activeTag; }

        void setActiveTag(const QString &activeTag);

        std::vector<LibraryEntry *> entries() const;

        QStringList tags() const;

        void addEntry(std::unique_ptr<LibraryEntry> entry);

        LibraryEntry *findById(const QUuid &id);

    private:

        std::vector<std::unique_ptr<LibraryEntry>> _entries;
        std::map<QString, size_t> _tags;
        QString _activeTag;

        void addTag(const QString &tag);

        void removeTag(const QString &tag);

        void removeEntry(LibraryEntry *entry);
    };

}

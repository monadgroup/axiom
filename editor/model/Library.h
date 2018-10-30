#pragma once

#include <QtCore/QString>
#include <QtCore/QUuid>
#include <functional>
#include <map>
#include <memory>
#include <vector>

#include "common/Event.h"

namespace AxiomModel {

    class LibraryEntry;

    class Project;

    class Library : public AxiomCommon::TrackedObject {
    public:
        enum class ConflictResolution { CANCEL, KEEP_OLD, KEEP_NEW, KEEP_BOTH };

        AxiomCommon::Event<LibraryEntry *> entryAdded;
        AxiomCommon::Event<const QString &> activeTagChanged;
        AxiomCommon::Event<const QString &> activeSearchChanged;
        AxiomCommon::Event<const QString &> tagAdded;
        AxiomCommon::Event<const QString &> tagRemoved;
        AxiomCommon::Event<> changed;

        Library();

        Library(QString activeTag, QString activeSearch, std::vector<std::unique_ptr<LibraryEntry>> entries);

        ~Library() override;

        void import(Library *library,
                    const std::function<ConflictResolution(LibraryEntry *, LibraryEntry *)> &resolveConflict);

        const QString &activeTag() const { return _activeTag; }

        void setActiveTag(const QString &activeTag);

        const QString &activeSearch() const { return _activeSearch; }

        void setActiveSearch(const QString &search);

        std::vector<LibraryEntry *> entries() const;

        QStringList tags() const;

        void addEntry(std::unique_ptr<LibraryEntry> entry);

        LibraryEntry *findById(const QUuid &id);

    private:
        std::vector<std::unique_ptr<LibraryEntry>> _entries;
        std::map<QString, size_t> _tags;
        QString _activeTag;
        QString _activeSearch;

        void addTag(const QString &tag);

        void removeTag(const QString &tag);

        void removeEntry(LibraryEntry *entry);
    };
}

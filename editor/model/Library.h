#pragma once

#include <memory>
#include <vector>
#include <map>
#include <QtCore/QObject>
#include <QtCore/QString>

namespace AxiomModel {

    class Project;

    class LibraryEntry;

    class Library : public QObject {
    Q_OBJECT

    public:
        Library(Project *project);

        ~Library();

        const QString &activeTag() const { return _activeTag; }

        void serialize(QDataStream &stream) const;

        void deserialize(QDataStream &stream);

        void clear();

        std::vector<std::unique_ptr<LibraryEntry>> &entries() { return _entries; }

        std::map<QString, size_t> &tags() { return _tags; }

        void addEntry(std::unique_ptr<LibraryEntry> entry);

    public slots:

        void setActiveTag(const QString &activeTag);

    signals:

        void entryAdded(LibraryEntry *entry);

        void activeTagChanged(const QString &activeTag);

        void tagAdded(const QString &tag);

        void tagRemoved(const QString &tag);

    private slots:

        void addTag(const QString &tag);

        void removeTag(const QString &tag);

        void removeEntry(LibraryEntry *entry);

    private:

        Project *project;
        std::vector<std::unique_ptr<LibraryEntry>> _entries;
        std::map<QString, size_t> _tags;
        QString _activeTag;
    };

}

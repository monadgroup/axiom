#pragma once

#include <memory>
#include <vector>
#include <map>
#include <QtCore/QObject>
#include <QtCore/QString>

namespace AxiomModel {

    class LibraryEntry;

    class Library : public QObject {
        Q_OBJECT

    public:
        ~Library();

        void serialize(QDataStream &stream) const;

        void deserialize(QDataStream &stream);

        void clear();

        std::vector<std::unique_ptr<LibraryEntry>> &entries() { return _entries; }

        void addEntry(std::unique_ptr<LibraryEntry> entry);

    signals:

        void entryAdded(LibraryEntry *entry);

    private:

        std::vector<std::unique_ptr<LibraryEntry>> _entries;
    };

}

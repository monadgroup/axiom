#pragma once

#include <memory>
#include <vector>
#include <map>
#include <QtCore/QObject>
#include <QtCore/QString>

#include "LibraryEntry.h"

namespace AxiomModel {

    class Library {
    public:
        std::vector<std::unique_ptr<LibraryEntry>> entries;
        std::map<QString, std::vector<LibraryEntry *>> index;

        void serialize(QDataStream &stream) const;

        void deserialize(QDataStream &stream);

        void clear();
    };

}

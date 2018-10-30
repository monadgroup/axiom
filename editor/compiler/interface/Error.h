#pragma once

#include <QtCore/QString>

#include <string>

#include "Frontend.h"
#include "OwnedObject.h"

namespace MaximCompiler {

    class Error : public OwnedObject {
    public:
        Error();

        explicit Error(void *handle);

        QString getDescription() const;

        MaximFrontend::SourceRange getRange() const;
    };
}

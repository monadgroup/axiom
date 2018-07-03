#pragma once

#include <QtCore/QString>

#include <string>

#include "OwnedObject.h"
#include "Frontend.h"

namespace MaximCompiler {

    class Error : public OwnedObject {
    public:
        explicit Error(void *handle);

        QString getDescription() const;

        MaximFrontend::SourceRange getRange() const;
    };

}

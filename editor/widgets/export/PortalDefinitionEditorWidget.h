#pragma once

#include <QtCore/QSet>
#include <QtWidgets/QScrollArea>
#include <vector>

class QLineEdit;

namespace AxiomModel {
    class Project;
}

namespace AxiomBackend {
    class AudioConfiguration;
}

namespace AxiomGui {

    class PortalDefinitionEditorWidget : public QScrollArea {
        Q_OBJECT

    public:
        explicit PortalDefinitionEditorWidget(const AxiomModel::Project &project,
                                              const AxiomBackend::AudioConfiguration &configuration);

        std::vector<QString> getNames() const;

    public slots:

        void setInstrumentPrefix(const QString &oldSafePrefix, const QString &newSafePrefix);

    private slots:

        void ensureLineSafe(size_t index);

    private:
        struct LineEdit {
            QString lastSafeName;
            QLineEdit *lineEdit;
        };

        QSet<QString> usedNames;
        std::vector<LineEdit> lineEdits;
    };
}

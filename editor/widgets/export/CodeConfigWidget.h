#pragma once

#include <QtWidgets/QWidget>

class QComboBox;
class QRadioButton;

namespace AxiomGui {

    class CodeConfigWidget : public QWidget {
        Q_OBJECT

    public:
        CodeConfigWidget();

    private:
        QComboBox *optimizationSelect;

        QRadioButton *instrumentAndLibraryContent;
        QRadioButton *instrumentContent;
        QRadioButton *libraryContent;
    };
}

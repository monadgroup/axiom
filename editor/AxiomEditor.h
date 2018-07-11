#include "AxiomApplication.h"
#include "widgets/windows/MainWindow.h"

namespace AxiomBackend {
    class AudioBackend;
}

class AxiomEditor {
public:
    AxiomEditor(AxiomApplication *application, AxiomBackend::AudioBackend *backend);

    // There are two ways to run the editor GUI:
    //  1. Call `run` on the UI thread, which will show the editor and start an event loop on the thread. The
    //     function returns when the editor is closed. This is useful for standalone backends.
    //  2. Call `show` and `hide` on the UI thread to show/hide the editor respectively. Call `idle` to run the
    //     editor's event loop.

    // Runs the editor, taking control of the current thread. Should be called from the UI thread.
    int run();

    // Shows/hides the editor window. Should be called from the UI thread. Todo: allow showing inside a provided
    // window?
    void show();
    void hide();

    // Runs the editor window's event loop.
    void idle();

    // Used internally. Not stable APIs.
    AxiomGui::MainWindow *window() { return &_window; }

private:
    AxiomGui::MainWindow _window;
};

#include "AxiomVstEditor.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "AxiomVstPlugin.h"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

AxiomVstEditor::AxiomVstEditor(AxiomApplication *application, AxiomVstPlugin *plugin)
    : editorSize({0, 0, WINDOW_HEIGHT, WINDOW_WIDTH}), editor(application, &plugin->backend()), plugin(plugin) {
    editor.window()->setResizerVisible(true);
    editor.window()->resize(WINDOW_WIDTH, WINDOW_HEIGHT);

    QObject::connect(editor.window(), &AxiomGui::MainWindow::resized, [this](QSize newSize) { resize(newSize); });
}

bool AxiomVstEditor::getRect(ERect **rect) {
    *rect = &editorSize;
    return true;
}

bool AxiomVstEditor::open(void *platformWindow) {
    AEffEditor::open(platformWindow);

#ifdef Q_OS_WIN
    auto window = editor.window();

    window->setWindowFlags(window->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    window->setAttribute(Qt::WA_NativeWindow, true);

    HWND hwnd = (HWND) window->winId();

    ::SetWindowLong(hwnd, GWL_STYLE, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
    ::SetParent(hwnd, (HWND) platformWindow);
    ::SetWindowPos(hwnd, HWND_TOP, 0, 0, editorSize.right, editorSize.bottom, SWP_SHOWWINDOW);
    ::ShowWindow(hwnd, SW_SHOW);

    window->setVisible(true);
    window->move(0, 0);
    window->resize(editorSize.right, editorSize.bottom);
    window->update();
#else
    editor.show();
#endif

    return true;
}

void AxiomVstEditor::close() {
#ifdef Q_OS_WIN
    HWND hwnd = (HWND) editor.window()->winId();
    ::ShowWindow(hwnd, SW_HIDE);
    ::SetParent(hwnd, nullptr);
#else
    editor.hide();
#endif

    AEffEditor::close();
}

void AxiomVstEditor::idle() {
    editor.idle();
}

void AxiomVstEditor::resize(QSize newSize) {
    if (newSize.width() < editor.window()->minimumWidth()) newSize.setWidth(editor.window()->minimumWidth());
    if (newSize.height() < editor.window()->minimumHeight()) newSize.setHeight(editor.window()->minimumHeight());

    editor.window()->resize(newSize);
    editorSize.right = newSize.width();
    editorSize.bottom = newSize.height();
    plugin->sizeWindow(newSize.width(), newSize.height());
}

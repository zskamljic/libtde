#pragma once

#include <QDialog>

#include <optional>

class QPushButton;
class QVBoxLayout;

namespace tde {

// A dialog with the same frameless look as the main windows: a header bar with a centred
// title and the close button where the desktop config puts window buttons.
class Dialog : public QDialog {
    Q_OBJECT

public:
    explicit Dialog(const QString& title, QWidget* parent = nullptr);

    QVBoxLayout* contentLayout() const { return m_content; }

    // Like exec(): shows the dialog and waits until it is closed, keeping other windows from
    // taking input meanwhile, but without asking the compositor for a modal window.
    int run();

    // The button Enter presses. Call it once the button is inside the dialog.
    void setDefaultButton(QPushButton* button);

    // Asks for a line of text; nullopt when cancelled. The first `selectionLength` characters
    // start selected, all of them by default.
    static std::optional<QString> getText(QWidget* parent, const QString& title, const QString& label,
        const QString& text, const QString& acceptLabel, qsizetype selectionLength = -1);

    // Asks before doing something; Cancel is the default button. A destructive action is
    // shown in red.
    static bool confirm(QWidget* parent, const QString& title, const QString& message, const QString& detail,
        const QString& acceptLabel, bool destructive = true);

    // The application's icon, `name`, its version and `description`.
    static void showAbout(QWidget* parent, const QString& name, const QString& description);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QVBoxLayout* m_content;
};

} // namespace tde

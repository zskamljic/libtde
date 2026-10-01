#include "tde/Dialog.hpp"

#include "tde/DesktopConfig.hpp"
#include "tde/FramelessHelper.hpp"
#include "tde/HeaderBar.hpp"
#include "tde/WindowButtons.hpp"

#include <QApplication>
#include <QDialogButtonBox>
#include <QEventLoop>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <algorithm>

using namespace Qt::StringLiterals;

namespace tde {
namespace {

// While a dialog runs, swallows input meant for other windows: the dialog is modal within
// the application, but the compositor sees an ordinary child window, so GNOME Shell does
// not attach and dim it.
class ModalGuard : public QObject {
public:
    explicit ModalGuard(QWidget* dialog)
        : m_dialog(dialog)
    {
        qApp->installEventFilter(this);
    }

    ~ModalGuard() override { qApp->removeEventFilter(this); }

protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        switch (event->type()) {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::Wheel:
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        case QEvent::ShortcutOverride:
        case QEvent::ContextMenu:
        case QEvent::Close:
        case QEvent::DragEnter:
        case QEvent::Drop:
            if (auto* widget = qobject_cast<QWidget*>(watched); widget && !belongsToDialog(widget)) {
                if (event->type() == QEvent::Close)
                    event->ignore();
                return true;
            }
            break;
        default:
            break;
        }
        return false;
    }

private:
    // The dialog itself, or a popup opened from it, like a text field's context menu.
    bool belongsToDialog(const QWidget* widget) const
    {
        for (const QWidget* w = widget; w; w = w->parentWidget()) {
            if (w == m_dialog)
                return true;
        }
        return false;
    }

    QWidget* m_dialog;
};

} // namespace

void Dialog::setDefaultButton(QPushButton* button)
{
    // Enter presses the button with the focus, or this one while no button has it.
    for (QPushButton* other : findChildren<QPushButton*>())
        other->setAutoDefault(true);
    button->setDefault(true);
}

bool Dialog::eventFilter(QObject* watched, QEvent* event)
{
    // Left and right move between the buttons, in the order they are shown.
    if (event->type() == QEvent::KeyPress && qobject_cast<QPushButton*>(watched)) {
        const int key = static_cast<QKeyEvent*>(event)->key();
        if (key == Qt::Key_Left || key == Qt::Key_Right) {
            QList<QPushButton*> buttons;
            for (QPushButton* button : findChildren<QPushButton*>()) {
                if (button->isVisible() && button->isEnabled() && button->focusPolicy() != Qt::NoFocus)
                    buttons << button;
            }
            std::ranges::sort(buttons, {}, [](const QPushButton* b) { return b->mapToGlobal(QPoint()).x(); });
            const qsizetype at = buttons.indexOf(static_cast<QPushButton*>(watched));
            const qsizetype next = at + (key == Qt::Key_Right ? 1 : -1);
            if (at >= 0 && next >= 0 && next < buttons.size())
                buttons[next]->setFocus(Qt::TabFocusReason);
            return true;
        }
    }
    return QDialog::eventFilter(watched, event);
}

int Dialog::run()
{
    setWindowModality(Qt::NonModal);
    ModalGuard guard(this);
    QEventLoop loop;
    connect(this, &QDialog::finished, &loop, &QEventLoop::quit);
    for (QPushButton* button : findChildren<QPushButton*>())
        button->installEventFilter(this);
    show();
    raise();
    activateWindow();
    loop.exec();
    return result();
}

Dialog::Dialog(const QString& title, QWidget* parent)
    : QDialog(parent)
{
    setObjectName(u"Dialog"_s);
    setAttribute(Qt::WA_StyledBackground);
    setWindowTitle(title);
    new FramelessHelper(this);

    // Dialogs only get a close button, placed where the other windows have theirs.
    const auto& config = tde::desktop().windowButtons;
    std::vector<WindowButton> buttons;
    if (std::ranges::contains(config.order, WindowButton::Close))
        buttons.push_back(WindowButton::Close);

    auto* header = new HeaderBar(this);
    auto* windowButtons = new WindowButtons(buttons, header);
    auto* titleLabel = new QLabel(title, header);
    titleLabel->setObjectName(u"DialogTitle"_s);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    // An empty widget as wide as the buttons on the other side keeps the title centred.
    auto* balance = new QWidget(header);
    balance->setFixedWidth(windowButtons->sizeHint().width());
    const bool left = config.side == tde::ButtonSide::Left;
    QHBoxLayout* headerLayout = header->contentLayout();
    headerLayout->setContentsMargins(10, 6, 10, 7);
    headerLayout->addWidget(left ? windowButtons : balance);
    headerLayout->addWidget(titleLabel, 1);
    headerLayout->addWidget(left ? balance : windowButtons);

    auto* body = new QWidget(this);
    body->setObjectName(u"DialogBody"_s);
    body->setAttribute(Qt::WA_StyledBackground);
    m_content = new QVBoxLayout(body);
    m_content->setContentsMargins(20, 18, 20, 18);
    m_content->setSpacing(12);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(1, 1, 1, 1);
    layout->setSpacing(0);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->addWidget(header);
    layout->addWidget(body);
}

std::optional<QString> Dialog::getText(QWidget* parent, const QString& title, const QString& label, const QString& text,
    const QString& acceptLabel, qsizetype selectionLength)
{
    Dialog dialog(title, parent);
    auto* prompt = new QLabel(label);
    auto* entry = new QLineEdit(text);
    entry->setMinimumWidth(320);
    entry->setSelection(0, selectionLength < 0 ? text.size() : selectionLength);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Cancel);
    QPushButton* accept = buttons->addButton(acceptLabel, QDialogButtonBox::AcceptRole);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    dialog.contentLayout()->addWidget(prompt);
    dialog.contentLayout()->addWidget(entry);
    dialog.contentLayout()->addWidget(buttons);
    // Only once the buttons are inside the dialog does it learn which one is the default.
    dialog.setDefaultButton(accept);
    entry->setFocus();
    if (dialog.run() != QDialog::Accepted)
        return std::nullopt;
    return entry->text();
}

bool Dialog::confirm(QWidget* parent, const QString& title, const QString& message, const QString& detail,
    const QString& acceptLabel, bool destructive)
{
    Dialog dialog(title, parent);
    auto* heading = new QLabel(message);
    heading->setObjectName(u"ConfirmMessage"_s);
    heading->setWordWrap(true);
    heading->setMinimumWidth(340);
    heading->setMaximumWidth(460);
    auto* explanation = new QLabel(detail);
    explanation->setObjectName(u"AboutDetails"_s);
    explanation->setWordWrap(true);

    auto* buttons = new QDialogButtonBox;
    QPushButton* cancel = buttons->addButton(QDialogButtonBox::Cancel);
    QPushButton* accept = buttons->addButton(acceptLabel, QDialogButtonBox::AcceptRole);
    if (destructive)
        accept->setObjectName(u"DestructiveButton"_s);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    dialog.contentLayout()->addWidget(heading);
    dialog.contentLayout()->addWidget(explanation);
    dialog.contentLayout()->addSpacing(6);
    dialog.contentLayout()->addWidget(buttons);
    QPushButton* focused = destructive ? cancel : accept;
    dialog.setDefaultButton(focused);
    focused->setFocus();
    return dialog.run() == QDialog::Accepted;
}

void Dialog::showAbout(QWidget* parent, const QString& name, const QString& description)
{
    Dialog dialog(u"About %1"_s.arg(name), parent);

    auto* icon = new QLabel;
    icon->setPixmap(QApplication::windowIcon().pixmap(QSize(96, 96), dialog.devicePixelRatioF()));
    auto* title = new QLabel(name);
    title->setObjectName(u"AboutName"_s);
    auto* version = new QLabel(u"Version %1"_s.arg(QApplication::applicationVersion()));
    version->setObjectName(u"AboutDetails"_s);
    auto* text = new QLabel(description);

    QVBoxLayout* layout = dialog.contentLayout();
    layout->setContentsMargins(48, 24, 48, 32);
    layout->setSpacing(6);
    for (QLabel* label : {icon, title, version})
        layout->addWidget(label, 0, Qt::AlignHCenter);
    layout->addSpacing(10);
    layout->addWidget(text, 0, Qt::AlignHCenter);
    dialog.run();
}

} // namespace tde

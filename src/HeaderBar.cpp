#include "tde/HeaderBar.hpp"

#include "tde/DesktopConfig.hpp"
#include "tde/Theme.hpp"

#include <QApplication>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QToolButton>
#include <QWindow>

using namespace Qt::StringLiterals;

namespace tde {
namespace {

constexpr int NormalControl = 34; // pixels

} // namespace

HeaderBar::HeaderBar(QWidget* parent)
    : QWidget(parent)
    , m_layout(new QHBoxLayout(this))
{
    setObjectName(u"HeaderBar"_s);
    setAttribute(Qt::WA_StyledBackground);
    m_layout->setSpacing(6);
    applySize();
    connect(&desktopNotifier(), &DesktopNotifier::changed, this, &HeaderBar::applySize);
}

int HeaderBar::controlHeight()
{
    return desktop().titleBarSize == TitleBarSize::Compact ? 28 : NormalControl;
}

void HeaderBar::applySize()
{
    const int control = controlHeight();
    const bool compact = control < NormalControl;
    setFixedHeight(control + (compact ? 9 : 13));
    m_layout->setContentsMargins(6, compact ? 4 : 6, 6, compact ? 5 : 7);
    // Buttons, path bars and search fields have the height of a control, which applications
    // give them as a normal one has it; they are given the one there is now.
    for (QWidget* child : findChildren<QWidget*>()) {
        const int height = child->maximumHeight();
        if (child->minimumHeight() == height && (height == NormalControl || height == m_controlHeight))
            child->setFixedHeight(control);
        if (child->objectName() == u"HeaderButton")
            child->setMinimumWidth(control);
    }
    m_controlHeight = control;
}

void HeaderBar::showEvent(QShowEvent* event)
{
    // What the application put in since it was made.
    applySize();
    QWidget::showEvent(event);
}

QToolButton* HeaderBar::makeButton(const QString& iconName, const QString& toolTip, QWidget* parent)
{
    auto* button = new QToolButton(parent);
    button->setObjectName(u"HeaderButton"_s);
    button->setIcon(theme::symbolicIcon(iconName));
    button->setIconSize(QSize(16, 16));
    button->setToolTip(toolTip);
    button->setFocusPolicy(Qt::NoFocus);
    button->setFixedHeight(controlHeight());
    button->setMinimumWidth(controlHeight());
    return button;
}

void HeaderBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        m_pressPosition = event->position();
    event->accept();
}

void HeaderBar::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_pressPosition || !(event->buttons() & Qt::LeftButton))
        return;
    if ((event->position() - *m_pressPosition).manhattanLength() < QApplication::startDragDistance())
        return;
    m_pressPosition.reset();
    if (QWindow* handle = window()->windowHandle())
        handle->startSystemMove();
}

void HeaderBar::mouseReleaseEvent(QMouseEvent* event)
{
    m_pressPosition.reset();
    event->accept();
}

void HeaderBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    QWidget* top = window();
    if (event->button() != Qt::LeftButton || top->minimumSize() == top->maximumSize())
        return;
    top->isMaximized() ? top->showNormal() : top->showMaximized();
}

} // namespace tde

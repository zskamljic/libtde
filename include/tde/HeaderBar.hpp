#pragma once

#include <QPointF>
#include <QWidget>

#include <optional>

class QHBoxLayout;
class QToolButton;

namespace tde {

// A title bar replacement: holds buttons, moves the window when dragged and toggles
// maximization on double click.
class HeaderBar : public QWidget {
    Q_OBJECT

public:
    explicit HeaderBar(QWidget* parent = nullptr);

    QHBoxLayout* contentLayout() const { return m_layout; }

    static QToolButton* makeButton(const QString& iconName, const QString& toolTip, QWidget* parent);
    // How tall buttons and fields in header bars are, for the title bar size the desktop has.
    static int controlHeight();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    // Takes the height the desktop's config gives header bars, and gives what in it had the
    // height of a control before the height of one now.
    void applySize();

    QHBoxLayout* m_layout;
    int m_controlHeight = 0; // as it was last applied
    std::optional<QPointF> m_pressPosition;
};

} // namespace tde

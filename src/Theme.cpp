#include "tde/Theme.hpp"

#include "IconRepair.hpp"

#include <QAbstractItemView>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QIconEngine>
#include <QMenu>
#include <QPainter>
#include <QPalette>
#include <QProcess>
#include <QProxyStyle>
#include <QSettings>
#include <QSlider>
#include <QStandardPaths>
#include <QStyleFactory>
#include <QStyleHints>
#include <QStyleOption>
#include <QStyledItemDelegate>

#include <algorithm>
#include <cstdio>
#include <memory>
#include <print>

using namespace Qt::StringLiterals;

namespace tde::theme {
namespace {

Colors s_colors;
int s_cornerRadius = 5;

Colors arcDark()
{
    Colors c;
    c.window = QColor(0x383c4a);
    c.base = QColor(0x404552);
    c.header = QColor(0x2f343f);
    c.sidebar = QColor(0x353945);
    c.sidebarText = QColor(0xbac3cf);
    c.text = QColor(0xd3dae3);
    c.dimText = QColor(0x8a939f);
    c.accent = QColor(0x5294e2);
    c.accentText = QColor(0xffffff);
    c.border = QColor(0x2b2e39);
    c.hover = QColor(255, 255, 255, 18);
    c.pressed = QColor(255, 255, 255, 34);
    c.entry = QColor(0x383c4a);
    c.scrollbar = QColor(0x767b87);
    c.closeHover = QColor(0xcc575d);
    c.error = QColor(0xfc4138);
    return c;
}

Colors arcLight()
{
    Colors c;
    c.window = QColor(0xf5f6f7);
    c.base = QColor(0xffffff);
    c.header = QColor(0xe7e8eb);
    c.sidebar = QColor(0xf0f1f3);
    c.sidebarText = QColor(0x5c616c);
    c.text = QColor(0x5c616c);
    c.dimText = QColor(0xa0a4ab);
    c.accent = QColor(0x5294e2);
    c.accentText = QColor(0xffffff);
    c.border = QColor(0xd6d9de);
    c.hover = QColor(0, 0, 0, 13);
    c.pressed = QColor(0, 0, 0, 26);
    c.entry = QColor(0xffffff);
    c.scrollbar = QColor(0xb8babf);
    c.closeHover = QColor(0xf46067);
    c.error = QColor(0xfc4138);
    return c;
}

struct NamedColor {
    QStringView name;
    QColor Colors::* member;
};

const NamedColor namedColors[] = {
    {u"window", &Colors::window},
    {u"base", &Colors::base},
    {u"header", &Colors::header},
    {u"sidebar", &Colors::sidebar},
    {u"sidebar_text", &Colors::sidebarText},
    {u"text", &Colors::text},
    {u"dim_text", &Colors::dimText},
    {u"accent", &Colors::accent},
    {u"accent_text", &Colors::accentText},
    {u"border", &Colors::border},
    {u"hover", &Colors::hover},
    {u"pressed", &Colors::pressed},
    {u"entry", &Colors::entry},
    {u"scrollbar", &Colors::scrollbar},
    {u"close_hover", &Colors::closeHover},
    {u"error", &Colors::error},
};

// The look of the widgets all TDE applications share. `@name@` placeholders are replaced
// with the colours above, `@radius@` and its variants with the corner radii.
const QString BaseStyleSheet = uR"(
* { outline: 0; }
QWidget#HeaderBar { background: @header@; border-bottom: 1px solid @border@; }

QToolButton#HeaderButton {
    background: transparent; color: @text@;
    border: 1px solid transparent; border-radius: @radius@; padding: 5px;
}
QToolButton#HeaderButton:hover { background: @hover@; border-color: @border@; }
QToolButton#HeaderButton:pressed, QToolButton#HeaderButton:checked, QToolButton#HeaderButton:open {
    background: @pressed@; border-color: @border@;
}
QToolButton#HeaderButton:disabled { border-color: transparent; background: transparent; }
QToolButton#HeaderButton[popupMode="1"] { padding-right: 18px; }
QToolButton#HeaderButton::menu-button { border: none; width: 16px; }
QToolButton#HeaderButton::menu-indicator { image: none; width: 0; }

QHeaderView { background: @base@; border: none; }
QHeaderView::section {
    background: @base@; color: @dim_text@;
    border: none; border-bottom: 1px solid @border@; padding: 6px 8px;
}
QHeaderView::section:hover { color: @text@; }
QHeaderView::down-arrow, QHeaderView::up-arrow { width: 10px; height: 10px; }

QSplitter::handle { background: @border@; }

QMenu {
    background: @base@; color: @text@;
    border: 1px solid @border@; border-radius: @radius_large@; padding: 5px;
}
QMenu::item { padding: 6px 24px 6px 8px; border-radius: @radius_small@; background: transparent; }
QMenu::icon, QMenu::indicator { padding-left: 10px; }
QMenu::item:selected { background: @accent@; color: @accent_text@; }
QMenu::item:disabled { color: @dim_text@; }
QMenu::separator { height: 1px; background: @border@; margin: 5px 8px; }

QScrollBar:vertical { background: transparent; width: 10px; margin: 0; }
QScrollBar:horizontal { background: transparent; height: 10px; margin: 0; }
QScrollBar::handle { background: @scrollbar@; border-radius: @radius_scrollbar@; margin: 2px; }
QScrollBar::handle:vertical { min-height: 32px; }
QScrollBar::handle:horizontal { min-width: 32px; }
QScrollBar::handle:hover { background: @accent@; }
QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; border: none; background: none; }
QScrollBar::add-page, QScrollBar::sub-page { background: none; }

QToolTip {
    background: @header@; color: @text@;
    border: 1px solid @border@; border-radius: @radius@; padding: 4px 6px;
}

QDialog#Dialog { background: @border@; }
QWidget#DialogBody { background: @window@; }
QLabel#DialogTitle { font-weight: bold; }
QLabel#AboutName { font-size: 18pt; font-weight: bold; }
QLabel#AboutDetails { color: @dim_text@; }
QLineEdit {
    background: @fill@; color: @text@; border: 2px solid transparent; border-radius: @radius@; padding: 4px 6px;
    selection-background-color: @accent@; selection-color: @accent_text@;
}
QLineEdit:hover { background: @fill_hover@; }
QLineEdit:focus { border-color: @focus@; background: @fill@; }
QLineEdit:disabled { color: @dim_text@; }
QAbstractSpinBox QLineEdit, QComboBox QLineEdit, QAbstractSpinBox QLineEdit:hover, QComboBox QLineEdit:hover,
QAbstractSpinBox QLineEdit:focus, QComboBox QLineEdit:focus {
    background: transparent; border: none; padding: 0;
}

QComboBox QAbstractItemView { background: transparent; border: none; padding: 0; outline: 0; }
QPushButton {
    background: @header@; color: @text@;
    border: 1px solid @border@; border-radius: @radius_small@; padding: 6px 14px;
}
QPushButton:hover { background: @base@; }
QPushButton:default { background: @accent@; color: @accent_text@; border-color: @accent@; }
QPushButton#DestructiveButton { background: @error@; color: @accent_text@; border-color: @error@; }
QPushButton#DestructiveButton:hover { background: @close_hover@; }
QLabel#ConfirmMessage { font-weight: bold; }

QFrame#Toast { background: @header@; border: 1px solid @border@; border-radius: @radius_large@; }
QLabel#ToastText { color: @text@; padding: 3px 0; }
QPushButton#ToastButton {
    background: transparent; color: @accent@; border: none; font-weight: bold; padding: 4px 8px;
}
QPushButton#ToastButton:hover { background: @hover@; border-radius: @radius_small@; }
QProgressBar {
    background: @window@; border: none; border-radius: 3px; height: 6px; max-height: 6px;
}
QProgressBar::chunk { background: @accent@; border-radius: 3px; }
)"_s;

QString s_applicationStyleSheet;

QString cssColor(const QColor& color)
{
    if (color.alpha() == 255)
        return color.name();
    return u"rgba(%1, %2, %3, %4)"_s.arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha());
}

QColor withAlpha(QColor color, int alpha)
{
    color.setAlpha(alpha);
    return color;
}

// The background of entries and spin boxes, a shade of the text over whatever is below.
QColor entryFill(const Colors& colors, bool hovered)
{
    return withAlpha(colors.text, hovered ? 30 : 20);
}

QColor focusRing(const Colors& colors)
{
    return withAlpha(colors.accent, 170);
}

QString expanded(QString sheet, const Colors& colors)
{
    for (const auto& [name, member] : namedColors)
        sheet.replace(u'@' + name.toString() + u'@', cssColor(colors.*member));
    // Entries are filled in rather than outlined, as GNOME's are.
    sheet.replace(u"@fill@"_s, cssColor(entryFill(colors, false)));
    sheet.replace(u"@fill_hover@"_s, cssColor(entryFill(colors, true)));
    sheet.replace(u"@focus@"_s, cssColor(focusRing(colors)));
    const auto px = [](int value) { return u"%1px"_s.arg(value); };
    sheet.replace(u"@radius@"_s, px(radius(RadiusSize::Normal)));
    sheet.replace(u"@radius_small@"_s, px(radius(RadiusSize::Small)));
    sheet.replace(u"@radius_large@"_s, px(radius(RadiusSize::Large)));
    sheet.replace(u"@radius_scrollbar@"_s, px(std::min(radius(RadiusSize::Normal), 3)));
    return sheet;
}

QString styleSheet(const Colors& colors)
{
    return expanded(BaseStyleSheet, colors) + expanded(s_applicationStyleSheet, colors);
}

QPalette palette(const Colors& c)
{
    QPalette p;
    p.setColor(QPalette::Window, c.window);
    p.setColor(QPalette::WindowText, c.text);
    p.setColor(QPalette::Base, c.base);
    p.setColor(QPalette::AlternateBase, c.window);
    p.setColor(QPalette::Text, c.text);
    p.setColor(QPalette::Button, c.header);
    p.setColor(QPalette::ButtonText, c.text);
    p.setColor(QPalette::BrightText, c.accentText);
    p.setColor(QPalette::Highlight, c.accent);
    p.setColor(QPalette::HighlightedText, c.accentText);
    p.setColor(QPalette::ToolTipBase, c.header);
    p.setColor(QPalette::ToolTipText, c.text);
    p.setColor(QPalette::PlaceholderText, c.dimText);
    p.setColor(QPalette::Link, c.accent);
    p.setColor(QPalette::Light, c.base.lighter(115));
    p.setColor(QPalette::Midlight, c.base);
    p.setColor(QPalette::Mid, c.border);
    p.setColor(QPalette::Dark, c.border.darker(110));
    p.setColor(QPalette::Shadow, Qt::black);
    for (const auto role : {QPalette::WindowText, QPalette::Text, QPalette::ButtonText})
        p.setColor(QPalette::Disabled, role, c.dimText);
    return p;
}

// Sizes of the controls drawn after GNOME's below.
constexpr int ArrowWidth = 28; // the part of a drop-down with its arrow
constexpr int SpinButtonWidth = 32; // each of the - and + of a spin box
constexpr int ControlHeight = 34;
constexpr int SliderGroove = 4;
constexpr int SliderKnob = 20;

// A chevron pointing down, as GNOME's drop-downs have, or up.
void drawChevron(QPainter* painter, QPointF center, bool down, const QColor& color)
{
    constexpr qreal w = 4.0;
    constexpr qreal h = 2.25;
    const qreal dy = down ? h : -h;
    painter->setPen(QPen(color, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(Qt::NoBrush);
    painter->drawPolyline(
        QList<QPointF> {center + QPointF(-w, -dy), center + QPointF(0, dy), center + QPointF(w, -dy)});
}

void drawCheck(QPainter* painter, QPointF center, const QColor& color)
{
    painter->setPen(QPen(color, 1.75, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(Qt::NoBrush);
    painter->drawPolyline(
        QList<QPointF> {center + QPointF(-4.5, 0.5), center + QPointF(-1.5, 3.5), center + QPointF(4.5, -3.5)});
}

// The items of a drop-down's list: rounded where the pointer is, with a check by the one chosen.
class DropDownDelegate : public QStyledItemDelegate {
public:
    explicit DropDownDelegate(QComboBox* combo)
        : QStyledItemDelegate(combo)
        , m_combo(combo)
    {
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        const auto& c = colors();
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        if (index.data(Qt::AccessibleDescriptionRole).toString() == u"separator") {
            painter->fillRect(
                QRectF(option.rect.left() + 6, option.rect.center().y(), option.rect.width() - 12, 1), c.border);
            painter->restore();
            return;
        }
        const bool enabled = (option.state & QStyle::State_Enabled) && (index.flags() & Qt::ItemIsEnabled);
        const QRectF row = QRectF(option.rect).adjusted(0, 1, 0, -1);
        if (enabled && (option.state & (QStyle::State_MouseOver | QStyle::State_Selected))) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(option.state & QStyle::State_Sunken ? c.pressed : c.hover);
            const int r = radius(RadiusSize::Small);
            painter->drawRoundedRect(row, r, r);
        }
        QRectF text = row.adjusted(10, 0, -34, 0);
        if (const QIcon icon = index.data(Qt::DecorationRole).value<QIcon>(); !icon.isNull()) {
            const QSize size = option.decorationSize.isValid() ? option.decorationSize : QSize(16, 16);
            const QRect place(QPoint(int(text.left()), int(row.center().y() - size.height() / 2.0)), size);
            icon.paint(painter, place, Qt::AlignCenter, enabled ? QIcon::Normal : QIcon::Disabled);
            text.setLeft(text.left() + size.width() + 8);
        }
        painter->setFont(option.font);
        painter->setPen(enabled ? c.text : c.dimText);
        const QString label
            = option.fontMetrics.elidedText(index.data(Qt::DisplayRole).toString(), Qt::ElideRight, int(text.width()));
        painter->drawText(text, Qt::AlignVCenter | Qt::AlignLeft, label);
        if (index.row() == m_combo->currentIndex())
            drawCheck(painter, QPointF(row.right() - 18, row.center().y()), enabled ? c.text : c.dimText);
        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        if (index.data(Qt::AccessibleDescriptionRole).toString() == u"separator")
            return {10, 9};
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        return {size.width() + 10 + 34, std::max(size.height() + 12, 32)};
    }

private:
    QComboBox* m_combo;
};

class AppStyle : public QProxyStyle {
public:
    AppStyle()
        : QProxyStyle(QStyleFactory::create(u"Fusion"_s))
    {
    }

    int styleHint(
        StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const override
    {
        switch (hint) {
        case SH_ItemView_ActivateItemOnSingleClick:
        case SH_DialogButtonBox_ButtonsHaveIcons:
            return 0;
        case SH_ComboBox_Popup:
            return 0; // a list below it, as GNOME's drop-downs open, rather than a menu over it
        case SH_ComboBox_AllowWheelScrolling:
            return 0; // scrolling a page goes past them, rather than changing them
        case SH_DialogButtonLayout:
            return QDialogButtonBox::GnomeLayout; // Cancel on the left, the action on the right
        default:
            return QProxyStyle::styleHint(hint, option, widget, returnData);
        }
    }

    void polish(QWidget* widget) override
    {
        // Menus, tooltips and the lists of drop-downs are windows of their own; they need an
        // alpha channel for rounded corners.
        const bool popup = qobject_cast<QMenu*>(widget) || widget->windowType() == Qt::ToolTip
            || widget->objectName() == u"Popover" || widget->inherits("QComboBoxPrivateContainer");
        if (popup && !widget->testAttribute(Qt::WA_WState_Created)) {
            widget->setAttribute(Qt::WA_TranslucentBackground);
            widget->setWindowFlag(Qt::NoDropShadowWindowHint);
        }
        // Qt's own delegates draw the items as menus or plain lists; others are the program's.
        if (auto* combo = qobject_cast<QComboBox*>(widget)) {
            QAbstractItemDelegate* delegate = combo->itemDelegate();
            if (!delegate || delegate->inherits("QComboBoxDelegate") || delegate->inherits("QComboMenuDelegate"))
                combo->setItemDelegate(new DropDownDelegate(combo)); // the combo box owns it
            combo->view()->setMouseTracking(true);
        }
        if (auto* spin = qobject_cast<QAbstractSpinBox*>(widget)) {
            spin->setButtonSymbols(QAbstractSpinBox::PlusMinus);
            // Its entry, which takes the spin box's palette, shows the fill drawn below it through.
            QPalette palette = spin->palette();
            palette.setColor(QPalette::Base, Qt::transparent);
            spin->setPalette(palette);
        }
        QProxyStyle::polish(widget);
    }

    int pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const override
    {
        switch (metric) {
        case PM_SliderThickness:
        case PM_SliderControlThickness:
        case PM_SliderLength:
            return SliderKnob + 2;
        case PM_SliderTickmarkOffset:
            return 4;
        case PM_DefaultFrameWidth:
            // Room around the items of a drop-down's list.
            if (widget && widget->inherits("QComboBoxPrivateContainer"))
                return 5;
            return QProxyStyle::pixelMetric(metric, option, widget);
        default:
            return QProxyStyle::pixelMetric(metric, option, widget);
        }
    }

    QSize sizeFromContents(
        ContentsType type, const QStyleOption* option, const QSize& contents, const QWidget* widget) const override
    {
        switch (type) {
        case CT_ComboBox:
            return {contents.width() + 12 + ArrowWidth, std::max(contents.height() + 12, ControlHeight)};
        case CT_SpinBox:
            return {contents.width() + 12 + 2 * SpinButtonWidth, std::max(contents.height() + 12, ControlHeight)};
        default:
            return QProxyStyle::sizeFromContents(type, option, contents, widget);
        }
    }

    QRect subControlRect(
        ComplexControl control, const QStyleOptionComplex* option, SubControl sub, const QWidget* widget) const override
    {
        const QRect r = option->rect;
        if (control == CC_ComboBox) {
            switch (sub) {
            case SC_ComboBoxFrame:
                return r;
            case SC_ComboBoxArrow:
                return visualRect(
                    option->direction, r, QRect(r.right() - ArrowWidth + 1, r.top(), ArrowWidth, r.height()));
            case SC_ComboBoxEditField:
                return visualRect(option->direction, r, r.adjusted(10, 3, -ArrowWidth, -3));
            case SC_ComboBoxListBoxPopup:
                return r;
            default:
                break;
            }
        }
        if (control == CC_SpinBox) {
            switch (sub) {
            case SC_SpinBoxFrame:
                return r;
            case SC_SpinBoxUp:
                return visualRect(
                    option->direction, r, QRect(r.right() - SpinButtonWidth + 1, r.top(), SpinButtonWidth, r.height()));
            case SC_SpinBoxDown:
                return visualRect(option->direction, r,
                    QRect(r.right() - 2 * SpinButtonWidth + 1, r.top(), SpinButtonWidth, r.height()));
            case SC_SpinBoxEditField:
                return visualRect(option->direction, r, r.adjusted(10, 3, -2 * SpinButtonWidth - 4, -3));
            default:
                break;
            }
        }
        return QProxyStyle::subControlRect(control, option, sub, widget);
    }

    void drawComplexControl(ComplexControl control, const QStyleOptionComplex* option, QPainter* painter,
        const QWidget* widget) const override
    {
        if (const auto* combo = qstyleoption_cast<const QStyleOptionComboBox*>(option);
            combo && control == CC_ComboBox) {
            drawDropDown(*combo, painter, widget);
            return;
        }
        if (const auto* spin = qstyleoption_cast<const QStyleOptionSpinBox*>(option); spin && control == CC_SpinBox) {
            drawSpinBox(*spin, painter, widget);
            return;
        }
        if (const auto* slider = qstyleoption_cast<const QStyleOptionSlider*>(option); slider && control == CC_Slider) {
            drawSlider(*slider, painter, widget);
            return;
        }
        QProxyStyle::drawComplexControl(control, option, painter, widget);
    }

    void drawControl(
        ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const override
    {
        // The list of a drop-down, as a popover.
        if (element == CE_ShapedFrame && widget && widget->inherits("QComboBoxPrivateContainer")) {
            const auto& c = colors();
            const int r = radius(RadiusSize::Large);
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(c.border);
            painter->setBrush(c.base);
            painter->drawRoundedRect(QRectF(option->rect).adjusted(0.5, 0.5, -0.5, -0.5), r, r);
            painter->restore();
            return;
        }
        QProxyStyle::drawControl(element, option, painter, widget);
    }

private:
    // Like a button, with a chevron at the end; filled in like an entry when it can be typed in.
    void drawDropDown(const QStyleOptionComboBox& option, QPainter* painter, const QWidget* widget) const
    {
        const auto& c = colors();
        const bool enabled = option.state & State_Enabled;
        const bool hovered = enabled && (option.state & State_MouseOver);
        const bool open = option.state & (State_On | State_Sunken);
        const int r = radius(RadiusSize::Small);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        const QRectF frame = QRectF(option.rect).adjusted(0.5, 0.5, -0.5, -0.5);
        if (option.editable) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(entryFill(c, hovered));
            painter->drawRoundedRect(frame, radius(), radius());
            if (option.state & State_HasFocus)
                drawFocus(painter, QRectF(option.rect), radius());
        } else {
            painter->setPen(c.border);
            painter->setBrush(open || hovered ? c.base : c.header);
            painter->drawRoundedRect(frame, r, r);
            if (open) {
                painter->setBrush(c.hover);
                painter->drawRoundedRect(frame, r, r);
            }
            if ((option.state & State_HasFocus) && (option.state & State_KeyboardFocusChange))
                drawFocus(painter, QRectF(option.rect), r);
        }
        const QRect arrow = subControlRect(CC_ComboBox, &option, SC_ComboBoxArrow, widget);
        drawChevron(painter, QRectF(arrow).center() - QPointF(3, 0), true, enabled ? c.text : c.dimText);
        painter->restore();
    }

    // An entry with - and + at its end, as GNOME's spin buttons have them.
    void drawSpinBox(const QStyleOptionSpinBox& option, QPainter* painter, const QWidget* widget) const
    {
        const auto& c = colors();
        const bool enabled = option.state & State_Enabled;
        const int r = radius();
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(entryFill(c, enabled && (option.state & State_MouseOver)));
        painter->drawRoundedRect(QRectF(option.rect), r, r);
        if (option.state & State_HasFocus)
            drawFocus(painter, QRectF(option.rect), r);

        const auto button = [&](SubControl sub, QAbstractSpinBox::StepEnabledFlag step, bool plus) {
            const QRectF place = subControlRect(CC_SpinBox, &option, sub, widget);
            const bool can = enabled && (option.stepEnabled & step);
            if (can && (option.activeSubControls & sub)) {
                painter->setPen(Qt::NoPen);
                painter->setBrush((option.state & State_Sunken) ? c.pressed : c.hover);
                painter->drawRoundedRect(place.adjusted(3, 3, -3, -3), r, r);
            }
            painter->setPen(QPen(can ? c.text : c.dimText, 1.5, Qt::SolidLine, Qt::RoundCap));
            const QPointF center = place.center();
            painter->drawLine(center + QPointF(-5, 0), center + QPointF(5, 0));
            if (plus)
                painter->drawLine(center + QPointF(0, -5), center + QPointF(0, 5));
            // A line before each button, as between the parts of GNOME's.
            painter->fillRect(QRectF(place.left(), place.top() + 7, 1, place.height() - 14), withAlpha(c.text, 30));
        };
        if (option.buttonSymbols != QAbstractSpinBox::NoButtons) {
            button(SC_SpinBoxDown, QAbstractSpinBox::StepDownEnabled, false);
            button(SC_SpinBoxUp, QAbstractSpinBox::StepUpEnabled, true);
        }
        painter->restore();
    }

    // A thin trough filled in up to a round knob, with marks beside it when asked for.
    void drawSlider(const QStyleOptionSlider& option, QPainter* painter, const QWidget* widget) const
    {
        const auto& c = colors();
        const bool enabled = option.state & State_Enabled;
        const bool horizontal = option.orientation == Qt::Horizontal;
        const QRect groove = QProxyStyle::subControlRect(CC_Slider, &option, SC_SliderGroove, widget);
        const QRect handle = QProxyStyle::subControlRect(CC_Slider, &option, SC_SliderHandle, widget);
        const QPointF knob = QRectF(handle).center();
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);

        // The trough reaches as far as the knob's middle can go.
        const qreal inset = SliderKnob / 2.0 + 1;
        QRectF trough;
        QRectF filled;
        if (horizontal) {
            const qreal y = knob.y() - SliderGroove / 2.0;
            trough = QRectF(groove.left() + inset, y, groove.width() - 2 * inset, SliderGroove);
            filled = option.upsideDown ? QRectF(knob.x(), y, trough.right() - knob.x(), SliderGroove)
                                       : QRectF(trough.left(), y, knob.x() - trough.left(), SliderGroove);
        } else {
            const qreal x = knob.x() - SliderGroove / 2.0;
            trough = QRectF(x, groove.top() + inset, SliderGroove, groove.height() - 2 * inset);
            filled = option.upsideDown ? QRectF(x, trough.top(), SliderGroove, knob.y() - trough.top())
                                       : QRectF(x, knob.y(), SliderGroove, trough.bottom() - knob.y());
        }
        constexpr qreal round = SliderGroove / 2.0;
        painter->setBrush(withAlpha(c.text, 45));
        painter->drawRoundedRect(trough, round, round);
        painter->setBrush(enabled ? c.accent : c.dimText);
        painter->drawRoundedRect(filled, round, round);

        if (option.tickPosition != QSlider::NoTicks && option.maximum > option.minimum) {
            const int interval = option.tickInterval > 0 ? option.tickInterval : std::max(option.pageStep, 1);
            painter->setBrush(withAlpha(c.text, 90));
            const int span = int(horizontal ? trough.width() : trough.height());
            for (int value = option.minimum; value <= option.maximum; value += interval) {
                const int at = sliderPositionFromValue(option.minimum, option.maximum, value, span, option.upsideDown);
                const auto mark = [&](qreal offset) {
                    painter->drawRoundedRect(horizontal ? QRectF(trough.left() + at - 0.75, knob.y() + offset, 1.5, 5)
                                                        : QRectF(knob.x() + offset, trough.top() + at - 0.75, 5, 1.5),
                        0.75, 0.75);
                };
                if (option.tickPosition & QSlider::TicksBelow)
                    mark(SliderKnob / 2.0 + 1);
                if (option.tickPosition & QSlider::TicksAbove)
                    mark(-SliderKnob / 2.0 - 6);
            }
        }

        // The knob, white in either theme, with a soft shadow below.
        const qreal knobRadius = SliderKnob / 2.0;
        painter->setBrush(QColor(0, 0, 0, 40));
        painter->drawEllipse(knob + QPointF(0, 1), knobRadius + 0.5, knobRadius + 0.5);
        const bool active = enabled && (option.state & State_MouseOver) && (option.activeSubControls & SC_SliderHandle);
        painter->setBrush(!enabled                        ? c.dimText
                : active || (option.state & State_Sunken) ? QColor(Qt::white)
                                                          : QColor(0xf6f5f4));
        painter->setPen(QPen(QColor(0, 0, 0, 50), 1));
        painter->drawEllipse(knob, knobRadius - 0.5, knobRadius - 0.5);
        if ((option.state & State_HasFocus) && (option.state & State_KeyboardFocusChange)) {
            painter->setPen(QPen(focusRing(c), 2));
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(knob, knobRadius + 1.5, knobRadius + 1.5);
        }
        painter->restore();
    }

    static void drawFocus(QPainter* painter, QRectF rect, int r)
    {
        painter->setPen(QPen(focusRing(colors()), 2));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(rect.adjusted(1, 1, -1, -1), std::max(0, r - 1), std::max(0, r - 1));
    }
};

// Recolours a (symbolic) icon at paint time, so it follows the current colours.
class SymbolicIconEngine : public QIconEngine {
public:
    explicit SymbolicIconEngine(QIcon source)
        : m_source(std::move(source))
    {
    }

    void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) override
    {
        const qreal scale = painter->device() ? painter->device()->devicePixelRatio() : 1.0;
        painter->drawPixmap(rect, scaledPixmap(rect.size(), mode, state, scale));
    }

    QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) override
    {
        return scaledPixmap(size, mode, state, 1.0);
    }

    QPixmap scaledPixmap(const QSize& size, QIcon::Mode mode, QIcon::State state, qreal scale) override
    {
        QPixmap pixmap = m_source.pixmap(size, scale, QIcon::Normal, state);
        if (pixmap.isNull())
            return pixmap;

        QColor color = mode == QIcon::Selected ? colors().accentText : colors().text;
        if (mode == QIcon::Disabled)
            color.setAlphaF(0.4f);

        QPainter painter(&pixmap);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(QRect(QPoint(), pixmap.size()), color);
        return pixmap;
    }

    QSize actualSize(const QSize& size, QIcon::Mode mode, QIcon::State state) override
    {
        return m_source.actualSize(size, mode, state);
    }

    QIconEngine* clone() const override { return std::make_unique<SymbolicIconEngine>(m_source).release(); }
    QString key() const override { return u"tde-symbolic"_s; }

private:
    QIcon m_source;
};

// The icon theme GNOME and GTK programs are set to use, which the settings of TDE set too.
QString gnomeIconTheme()
{
    QProcess gsettings;
    gsettings.setStandardInputFile(QProcess::nullDevice());
    gsettings.setStandardErrorFile(QProcess::nullDevice());
    gsettings.start(u"gsettings"_s, {u"get"_s, u"org.gnome.desktop.interface"_s, u"icon-theme"_s});
    if (!gsettings.waitForFinished(2000) || gsettings.exitStatus() != QProcess::NormalExit || gsettings.exitCode() != 0)
        return {};
    // Printed as a GVariant string: in single quotes.
    QString name = QString::fromUtf8(gsettings.readAllStandardOutput()).trimmed();
    if (name.size() >= 2 && name.front() == u'\'' && name.back() == u'\'')
        name = name.mid(1, name.size() - 2);
    return name;
}

QString gtkIconTheme()
{
    const QString configDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    for (const auto* gtk : {"gtk-4.0", "gtk-3.0"}) {
        const QSettings settings(u"%1/%2/settings.ini"_s.arg(configDir, QLatin1StringView(gtk)), QSettings::IniFormat);
        const QString name = settings.value(u"Settings/gtk-icon-theme-name"_s).toString();
        if (!name.isEmpty())
            return name;
    }
    return {};
}

void setupIconTheme(const QString& configured)
{
    // Without a desktop platform theme Qt only searches its resources.
    QStringList searchPaths = QIcon::themeSearchPaths();
    QStringList candidates {QDir::homePath() + u"/.icons"_s};
    for (const QString& dataDir : QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation))
        candidates << dataDir + u"/icons"_s;
    for (const QString& path : std::as_const(candidates)) {
        if (!searchPaths.contains(path) && QFileInfo(path).isDir())
            searchPaths << path;
    }
    // Icons repaired for Qt's SVG renderer come before the originals.
    const QString repaired = repairedIconsRoot();
    searchPaths.removeAll(repaired);
    const QStringList themePaths = searchPaths;
    searchPaths.prepend(repaired);
    QIcon::setThemeSearchPaths(searchPaths);

    // Without one configured, the one the rest of the desktop uses: in TDE, the one its settings
    // gave GNOME and GTK programs; elsewhere, what Qt's platform theme knows of first.
    QString name = configured;
    const bool tde = qEnvironmentVariable("XDG_CURRENT_DESKTOP").split(u':').contains(u"TDE"_s, Qt::CaseInsensitive);
    if (name.isEmpty() && tde)
        name = gnomeIconTheme();
    if (name.isEmpty()) {
        const QString platform = QIcon::themeName();
        if (!platform.isEmpty() && platform != u"hicolor")
            name = platform;
    }
    if (name.isEmpty())
        name = gtkIconTheme();
    if (name.isEmpty() && !tde)
        name = gnomeIconTheme();
    if (name.isEmpty())
        name = u"Adwaita"_s;
    QIcon::setThemeName(name);
    QIcon::setFallbackThemeName(u"Adwaita"_s);
    repairIconsInBackground({name, u"Adwaita"_s, u"hicolor"_s}, themePaths);
}

} // namespace

const Colors& colors()
{
    return s_colors;
}

int radius(RadiusSize size)
{
    switch (size) {
    case RadiusSize::Small:
        return std::max(0, s_cornerRadius - 1);
    case RadiusSize::Normal:
        return s_cornerRadius;
    case RadiusSize::Large:
        return s_cornerRadius == 0 ? 0 : s_cornerRadius + 3;
    }
    return s_cornerRadius;
}

void setApplicationStyleSheet(const QString& sheet)
{
    s_applicationStyleSheet = sheet;
}

void apply(QApplication& app, const tde::DesktopConfig::Appearance& appearance)
{
    s_cornerRadius = appearance.cornerRadius;
    QString name = appearance.theme;
    if (name == u"system")
        name = app.styleHints()->colorScheme() == Qt::ColorScheme::Light ? u"arc"_s : u"arc-dark"_s;
    s_colors = name == u"arc" ? arcLight() : arcDark();

    for (auto it = appearance.colors.cbegin(); it != appearance.colors.cend(); ++it) {
        const auto named = std::ranges::find(namedColors, it.key(), &NamedColor::name);
        const QColor color = QColor::fromString(it.value());
        if (named == std::end(namedColors))
            std::println(stderr, "{}: unknown colour name \"{}\"", QCoreApplication::applicationName().toStdString(),
                it.key().toStdString());
        else if (!color.isValid())
            std::println(stderr, "{}: invalid colour \"{}\" for {}", QCoreApplication::applicationName().toStdString(),
                it.value().toStdString(), it.key().toStdString());
        else
            s_colors.*(named->member) = color;
    }

    setupIconTheme(appearance.iconTheme);
    QApplication::setStyle(std::make_unique<AppStyle>().release()); // the application owns it
    QApplication::setPalette(palette(s_colors));
    app.setStyleSheet(styleSheet(s_colors));
}

QIcon symbolicIcon(const QString& name)
{
    const QString symbolic = name.endsWith(u"-symbolic") ? name : name + u"-symbolic"_s;
    for (const QString& candidate : {symbolic, symbolic.chopped(9)}) {
        if (QIcon::hasThemeIcon(candidate))
            return QIcon(std::make_unique<SymbolicIconEngine>(QIcon::fromTheme(candidate)).release()); // QIcon owns it
    }
    return {};
}

QIcon themeIcon(std::initializer_list<QString> names)
{
    for (const QString& name : names) {
        if (QIcon::hasThemeIcon(name))
            return QIcon::fromTheme(name);
    }
    return {};
}

} // namespace tde::theme

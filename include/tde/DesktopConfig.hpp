#pragma once

#include <QHash>
#include <QString>
#include <QStringList>

#include <expected>

#include <vector>

namespace tde {

enum class ButtonSide { Left, Right };
enum class WindowButton { Minimize, Maximize, Close };

// Settings shared by all TDE applications, from ~/.config/tde/config.lua.
struct DesktopConfig {
    struct WindowButtons {
        ButtonSide side = ButtonSide::Right;
        std::vector<WindowButton> order {WindowButton::Minimize, WindowButton::Maximize, WindowButton::Close};

        bool operator==(const WindowButtons&) const = default;
    };

    struct Appearance {
        QString theme = QStringLiteral("arc-dark");
        QString iconTheme;
        QHash<QString, QString> colors;
        int cornerRadius = 5;

        bool operator==(const Appearance&) const = default;
    };

    struct Lock {
        int after = 5; // minutes without input before the screen locks; 0 for never

        bool operator==(const Lock&) const = default;
    };

    WindowButtons windowButtons;
    Appearance appearance;
    Lock lock;
    QString terminal; // program to open terminals with; empty picks one

    bool operator==(const DesktopConfig&) const = default;
};

QString desktopConfigPath();

// Reads the file at `path` over the settings in `config`; returns the warnings, or an error.
std::expected<QStringList, QString> readDesktopConfig(const QString& path, DesktopConfig& config);

// Writes the defaults, with comments on every setting, to `path` when there is no file there
// yet, so there is something to edit. Returns whether it wrote one. Applications call it
// once, when they start.
bool createDesktopConfig(const QString& path = desktopConfigPath());

// Loads the desktop config, reporting problems on stderr and using defaults for the rest.
DesktopConfig loadDesktopConfig(const QString& path = desktopConfigPath());

// The desktop config of this process: set at startup, and again when the file changes.
const DesktopConfig& desktop();
void setDesktop(DesktopConfig config);

} // namespace tde

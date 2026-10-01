# libtde

What the applications of TDE share, so they look and behave alike: the desktop-wide
configuration, the Arc theme, frameless windows with rounded corners and configurable window
buttons, header bars, dialogs and notifications.

Licensed under the GNU General Public License, version 3 or later; see [LICENSE](LICENSE).

## Configuration

`~/.config/tde/config.lua` holds the settings every TDE application follows: window button
placement and order, theme (`arc-dark`, `arc`, or `system` to follow the desktop's light or
dark preference), corner radius, icon theme, colour overrides and the terminal to open. See
[`data/config.lua`](data/config.lua); an installed copy is in
`/usr/share/doc/libtde/examples`. Applications keep their own settings next to it, in
`~/.config/tde/<application>/`, and apply changes to either as soon as a file is saved.

Icons are taken from the configured theme, falling back to Adwaita. Qt draws some SVG icons
with black patches (masks and clip paths, which Adwaita uses for shadows); when librsvg's
`rsvg-convert` is installed, those are drawn with it once and kept in `~/.cache/tde/icons`.

## Building

Needs a C++23 compiler, CMake ≥ 3.28, Qt ≥ 6.8 (base) and Lua 5.4 or 5.5.

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build
sudo cmake --install build
```

## Using it

```cmake
find_package(Tde 0.1 REQUIRED)
target_link_libraries(myapp PRIVATE Tde::Tde)
```

```cpp
#include <tde/DesktopConfig.hpp>
#include <tde/Theme.hpp>

tde::setDesktop(tde::loadDesktopConfig());
tde::theme::setApplicationStyleSheet(myStyleSheet); // the application's own widgets
tde::theme::apply(app, tde::desktop().appearance);
```

A window becomes a TDE window with a `tde::FramelessHelper`, a `tde::HeaderBar` and
`tde::WindowButtons`; `tde::Dialog`, `tde::Toast` and `tde::ConfigWatcher` cover dialogs,
notifications and following config edits. Until 1.0, each minor version may change the ABI.

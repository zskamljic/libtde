#pragma once

#include <QString>
#include <QStringList>

namespace tde::theme {

// Qt's SVG renderer gets masks and clip paths wrong in ways newer icon themes rely on for
// shadows and cut-outs (Adwaita among them): such icons show up with black squares. librsvg,
// which those themes are made for, renders them right. Affected icons are rendered once with
// it into a cache that Qt searches before the themes themselves.

// Where the repaired icons are kept, as an icon theme folder: <root>/<theme>/<path in theme>.
QString repairedIconsRoot();

// Whether an SVG uses what Qt draws wrongly.
bool needsRepair(const QByteArray& svg);

// Repairs the icons of `themes` (with the themes they inherit) found under `searchPaths`, on a
// worker thread; once done, Qt is told to look them up again. Without librsvg, does nothing.
void repairIconsInBackground(const QStringList& themes, const QStringList& searchPaths);

} // namespace tde::theme

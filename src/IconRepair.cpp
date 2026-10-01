#include "IconRepair.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QImage>
#include <QProcess>
#include <QSaveFile>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QThreadPool>

using namespace Qt::StringLiterals;

namespace tde::theme {
namespace {

constexpr int RenderSize = 256;

// `theme` and everything it inherits, nearest first.
QStringList themeChain(const QStringList& themes, const QStringList& searchPaths)
{
    QStringList chain;
    QStringList pending = themes;
    while (!pending.isEmpty()) {
        const QString theme = pending.takeFirst();
        if (theme.isEmpty() || chain.contains(theme))
            continue;
        chain << theme;
        for (const QString& path : searchPaths) {
            const QString index = path + u'/' + theme + u"/index.theme"_s;
            if (!QFileInfo::exists(index))
                continue;
            const QSettings settings(index, QSettings::IniFormat);
            pending << settings.value(u"Icon Theme/Inherits"_s).toStringList();
            break;
        }
    }
    return chain;
}

// The SVG `source` drawn by librsvg, wrapped in an SVG Qt draws right: one embedded picture.
QByteArray rendered(const QString& rsvg, const QString& source, const QString& scratch)
{
    const QString png = scratch + u"/icon.png"_s;
    QProcess process;
    process.setStandardInputFile(QProcess::nullDevice());
    process.setStandardErrorFile(QProcess::nullDevice());
    process.start(rsvg,
        {u"--width"_s, QString::number(RenderSize), u"--height"_s, QString::number(RenderSize),
            u"--keep-aspect-ratio"_s, u"--output"_s, png, source});
    if (!process.waitForFinished(10'000) || process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
        return {};
    QFile file(png);
    if (!file.open(QIODevice::ReadOnly))
        return {};
    const QByteArray data = file.readAll();
    const QImage image = QImage::fromData(data);
    if (image.isNull())
        return {};
    const QByteArray width = QByteArray::number(image.width());
    const QByteArray height = QByteArray::number(image.height());
    return "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" width=\"" + width
        + "\" height=\"" + height + "\" viewBox=\"0 0 " + width + ' ' + height + "\"><image width=\"" + width
        + "\" height=\"" + height + "\" xlink:href=\"data:image/png;base64," + data.toBase64() + "\"/></svg>\n";
}

// Repairs what needs it; returns whether anything changed.
bool repair(const QStringList& themes, const QStringList& searchPaths)
{
    const QString rsvg = QStandardPaths::findExecutable(u"rsvg-convert"_s);
    if (rsvg.isEmpty())
        return false;
    const QTemporaryDir scratch;
    if (!scratch.isValid())
        return false;
    const QString root = repairedIconsRoot();

    bool changed = false;
    for (const QString& theme : themeChain(themes, searchPaths)) {
        for (const QString& path : searchPaths) {
            const QString themeDir = path + u'/' + theme;
            if (path == root || !QFileInfo(themeDir).isDir())
                continue;
            // Full-colour icons only: symbolic ones are drawn in one colour anyway.
            for (QDirIterator it(themeDir, {u"*.svg"_s}, QDir::Files, QDirIterator::Subdirectories); it.hasNext();) {
                const QString source = it.next();
                const QString relative = source.mid(themeDir.size() + 1);
                if (relative.contains(u"symbolic"_s))
                    continue;
                const QString target = root + u'/' + theme + u'/' + relative;
                const QFileInfo targetInfo(target);
                if (targetInfo.exists() && targetInfo.lastModified() >= QFileInfo(source).lastModified())
                    continue;
                QFile file(source);
                if (!file.open(QIODevice::ReadOnly) || !needsRepair(file.readAll()))
                    continue;
                const QByteArray svg = rendered(rsvg, source, scratch.path());
                if (svg.isEmpty())
                    continue;
                QDir().mkpath(targetInfo.absolutePath());
                QSaveFile output(target);
                if (output.open(QIODevice::WriteOnly) && output.write(svg) == svg.size() && output.commit())
                    changed = true;
            }
        }
    }
    return changed;
}

} // namespace

QString repairedIconsRoot()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + u"/tde/icons"_s;
}

bool needsRepair(const QByteArray& svg)
{
    return svg.contains("<mask") || svg.contains("<clipPath");
}

void repairIconsInBackground(const QStringList& themes, const QStringList& searchPaths)
{
    QThreadPool::globalInstance()->start([themes, searchPaths] {
        if (!repair(themes, searchPaths))
            return;
        // Icons are looked up anew once the search paths are set again.
        QMetaObject::invokeMethod(
            QCoreApplication::instance(), [] { QIcon::setThemeSearchPaths(QIcon::themeSearchPaths()); },
            Qt::QueuedConnection);
    });
}

} // namespace tde::theme

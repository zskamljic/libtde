#include "tde/ConfigWatcher.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>

using namespace Qt::StringLiterals;

namespace tde {
namespace {

QByteArray contentsOf(const QString& path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

} // namespace

ConfigWatcher::ConfigWatcher(const QStringList& paths, QObject* parent)
    : QObject(parent)
{
    for (const QString& path : paths)
        m_contents.insert(path, contentsOf(path));

    // Saving can take several steps (write a temporary file, rename it over); act once it is done.
    m_settle.setSingleShot(true);
    m_settle.setInterval(150);
    connect(&m_settle, &QTimer::timeout, this, &ConfigWatcher::check);
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, &m_settle, qOverload<>(&QTimer::start));
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, &m_settle, qOverload<>(&QTimer::start));
    watch();
}

void ConfigWatcher::watch()
{
    QStringList paths;
    for (auto it = m_contents.cbegin(); it != m_contents.cend(); ++it) {
        const QFileInfo info(it.key());
        if (info.exists())
            paths << info.absoluteFilePath();
        // The nearest existing folder, to notice the file (or its folder) being created.
        QDir directory = info.absoluteDir();
        while (!directory.exists() && directory.cdUp()) { }
        paths << directory.absolutePath();
    }
    paths.removeDuplicates();
    for (const QString& path : std::as_const(paths)) {
        if (!m_watcher.files().contains(path) && !m_watcher.directories().contains(path))
            m_watcher.addPath(path);
    }
}

void ConfigWatcher::check()
{
    watch(); // a replaced file is a new file, which must be watched again
    for (auto it = m_contents.begin(); it != m_contents.end(); ++it) {
        QByteArray contents = contentsOf(it.key());
        if (contents == it.value())
            continue;
        it.value() = std::move(contents);
        emit changed(it.key());
    }
}

} // namespace tde

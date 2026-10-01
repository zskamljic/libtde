#pragma once

#include <QByteArray>
#include <QFileSystemWatcher>
#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>

namespace tde {

// Tells when config files change, so running applications can apply them. Editors often save
// by replacing a file rather than writing to it, and a file may be created or deleted, so the
// folders are watched too; only a change in what a file holds counts.
class ConfigWatcher : public QObject {
    Q_OBJECT

public:
    explicit ConfigWatcher(const QStringList& paths, QObject* parent = nullptr);

signals:
    void changed(const QString& path);

private:
    void watch();
    void check();

    QFileSystemWatcher m_watcher;
    QTimer m_settle;
    QHash<QString, QByteArray> m_contents;
};

} // namespace tde

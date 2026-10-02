#include "tde/ConfigWatcher.hpp"
#include "tde/DesktopConfig.hpp"
#include "tde/LuaConfig.hpp"

#include <QFile>
#include <QSaveFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

using namespace Qt::StringLiterals;

class TestConfig : public QObject {
    Q_OBJECT

private:
    QString writeFile(const QString& name, const QByteArray& source)
    {
        const QString path = m_dir.filePath(name);
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
            qFatal("cannot write %s", qPrintable(path));
        file.write(source);
        return path;
    }

    QTemporaryDir m_dir;

private slots:
    void desktopDefaults()
    {
        tde::DesktopConfig config;
        const auto warnings = tde::readDesktopConfig(writeFile(u"tde.lua"_s, "return {}"), config);
        QVERIFY(warnings.has_value());
        QVERIFY(warnings->isEmpty());
        QCOMPARE(config.windowButtons.side, tde::ButtonSide::Right);
        QCOMPARE(config.windowButtons.order.size(), 3u);
        QCOMPARE(config.appearance.theme, u"arc-dark"_s);
        QCOMPARE(config.appearance.cornerRadius, 5);
        QCOMPARE(config.lock.after, 5);
    }

    void desktopConfig()
    {
        tde::DesktopConfig config;
        const auto warnings = tde::readDesktopConfig(
            writeFile(u"tde.lua"_s,
                "local radius = 4\n"
                "return {\n"
                "window_buttons = { position = \"left\", order = { \"close\", \"maximize\" } },\n"
                "appearance = { theme = \"arc\", icon_theme = \"Paper\", corner_radius = radius * 2,\n"
                "               colors = { accent = \"#ff0000\" } },\n"
                "lock = { after = 15 },\n"
                "}\n"),
            config);
        QVERIFY(warnings.has_value());
        QVERIFY2(warnings->isEmpty(), qPrintable(warnings->join(u'\n')));
        QCOMPARE(config.windowButtons.side, tde::ButtonSide::Left);
        QCOMPARE(config.windowButtons.order, (std::vector {tde::WindowButton::Close, tde::WindowButton::Maximize}));
        QCOMPARE(config.appearance.theme, u"arc"_s);
        QCOMPARE(config.appearance.iconTheme, u"Paper"_s);
        QCOMPARE(config.appearance.cornerRadius, 8);
        QCOMPARE(config.appearance.colors.value(u"accent"_s), u"#ff0000"_s);
        QCOMPARE(config.lock.after, 15);
    }

    void badValuesWarnAndKeepDefaults()
    {
        tde::DesktopConfig config;
        const auto warnings
            = tde::readDesktopConfig(writeFile(u"tde.lua"_s,
                                         "return {\n"
                                         "window_buttons = { position = \"top\", order = { \"close\", \"shade\" } },\n"
                                         "appearance = { corner_radius = 100, theme = 3 },\n"
                                         "lock = { after = -1 },\n"
                                         "}\n"),
                config);
        QVERIFY(warnings.has_value());
        QCOMPARE(warnings->size(), 5);
        QVERIFY(warnings->first().startsWith(u"window_buttons.position:"_s));
        QCOMPARE(config.windowButtons.side, tde::ButtonSide::Right);
        QCOMPARE(config.windowButtons.order, std::vector {tde::WindowButton::Close});
        QCOMPARE(config.appearance.cornerRadius, 5);
        QCOMPARE(config.appearance.theme, u"arc-dark"_s);
        QCOMPARE(config.lock.after, 5);
    }

    void createsDefaults()
    {
        const QString path = m_dir.filePath(u"fresh/tde/config.lua"_s);
        QVERIFY(tde::createDesktopConfig(path));

        // What it writes is the defaults, word for word, and reads without complaint.
        tde::DesktopConfig config;
        const auto warnings = tde::readDesktopConfig(path, config);
        QVERIFY(warnings);
        QVERIFY2(warnings->isEmpty(), qPrintable(warnings->join(u'\n')));
        QCOMPARE(config, tde::DesktopConfig {});

        // A file that is there is left as it is.
        writeFile(u"fresh/tde/config.lua"_s, "return { appearance = { corner_radius = 0 } }");
        QVERIFY(!tde::createDesktopConfig(path));
        QFile file(path);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(file.readAll(), QByteArray("return { appearance = { corner_radius = 0 } }"));
    }

    void lists()
    {
        QStringList names;
        QStringList single;
        bool wasString = true;
        const auto read = tde::readLuaConfig(writeFile(u"lists.lua"_s,
                                                 "return { items = { { name = \"a\" }, \"stray\", { name = \"b\" } }, "
                                                 "one = \"x\", many = { \"y\", \"z\" } }"),
            [&](tde::LuaTableReader& reader) {
                reader.table("items", [&] {
                    reader.forEachArrayTable([&](int) { names << reader.string("name").value_or(QString()); });
                });
                single = reader.strings("one").value_or(QStringList());
                reader.strings("many", &wasString);
            });
        QVERIFY(read);
        QCOMPARE(names, (QStringList {u"a"_s, u"b"_s}));
        QCOMPARE(read->size(), 1); // the string where a table was expected
        QCOMPARE(single, QStringList {u"x"_s});
        QVERIFY(!wasString);
    }

    void watchesConfig()
    {
        const QString path = m_dir.filePath(u"watched/config.lua"_s);
        tde::ConfigWatcher watcher({path});
        QSignalSpy changed(&watcher, &tde::ConfigWatcher::changed);

        // Created, folder and all.
        QVERIFY(QDir().mkpath(m_dir.filePath(u"watched"_s)));
        writeFile(u"watched/config.lua"_s, "return {}");
        QTRY_COMPARE(changed.size(), 1);
        QCOMPARE(changed.first().first().toString(), path);

        // Replaced, as editors save.
        QSaveFile replaced(path);
        QVERIFY(replaced.open(QIODevice::WriteOnly));
        replaced.write("return { a = 1 }");
        QVERIFY(replaced.commit());
        QTRY_COMPARE(changed.size(), 2);

        // Another file in the folder, or the same contents again, are no change.
        writeFile(u"watched/state.lua"_s, "return {}");
        writeFile(u"watched/config.lua"_s, "return { a = 1 }");
        QTest::qWait(400);
        QCOMPARE(changed.size(), 2);
    }
};

QTEST_GUILESS_MAIN(TestConfig)
#include "tst_config.moc"

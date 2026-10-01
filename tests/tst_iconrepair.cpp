#include "IconRepair.hpp"

#include <QDir>
#include <QFile>
#include <QIcon>
#include <QImage>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

using namespace Qt::StringLiterals;
using namespace tde;

namespace {

// A green square with a shadow made 10% opaque through a mask, clipped and moved, the way
// Adwaita draws its shadows. Qt alone draws the clipped area as a black square.
const QByteArray maskedIcon
    = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"64\" height=\"64\" viewBox=\"0 0 64 64\">"
      "<filter id=\"f\" height=\"100%\" width=\"100%\" x=\"0%\" y=\"0%\">"
      "<feColorMatrix in=\"SourceGraphic\" type=\"matrix\" values=\"0 0 0 0 1 0 0 0 0 1 0 0 0 0 1 0 0 0 1 0\"/>"
      "</filter><mask id=\"m\"><g filter=\"url(#f)\"><rect fill-opacity=\"0.1\" height=\"64\" width=\"64\"/></g>"
      "</mask><clipPath id=\"c\"><rect height=\"76\" width=\"96\"/></clipPath>"
      "<rect x=\"8\" y=\"8\" width=\"32\" height=\"32\" fill=\"#2ec27e\"/>"
      "<g clip-path=\"url(#c)\" mask=\"url(#m)\" transform=\"matrix(1 0 0 1 -4 -8)\">"
      "<rect x=\"28\" y=\"32\" width=\"32\" height=\"32\"/></g></svg>";

// As icons are drawn: through Qt's SVG icon engine.
QImage render(const QString& path)
{
    return QIcon(path).pixmap(64, 64).toImage();
}

} // namespace

class TestIconRepair : public QObject {
    Q_OBJECT

    QTemporaryDir m_dir;

private slots:
    void detects()
    {
        QVERIFY(theme::needsRepair(maskedIcon));
        QVERIFY(theme::needsRepair("<svg><clipPath id=\"a\"/></svg>"));
        QVERIFY(!theme::needsRepair("<svg><rect width=\"4\" height=\"4\"/></svg>"));
    }

    void repairs()
    {
        if (QStandardPaths::findExecutable(u"rsvg-convert"_s).isEmpty())
            QSKIP("librsvg's rsvg-convert is not installed here");
        qputenv("XDG_CACHE_HOME", m_dir.filePath(u"cache"_s).toLocal8Bit());
        const QString icons = m_dir.filePath(u"icons"_s);
        QVERIFY(QDir().mkpath(icons + u"/Test/scalable/mimetypes"_s));
        QFile index(icons + u"/Test/index.theme"_s);
        QVERIFY(index.open(QIODevice::WriteOnly));
        index.write("[Icon Theme]\nName=Test\nDirectories=scalable/mimetypes\n");
        index.close();
        QFile icon(icons + u"/Test/scalable/mimetypes/shadowed.svg"_s);
        QVERIFY(icon.open(QIODevice::WriteOnly));
        icon.write(maskedIcon);
        icon.close();

        theme::repairIconsInBackground({u"Test"_s}, {icons});
        const QString repaired = theme::repairedIconsRoot() + u"/Test/scalable/mimetypes/shadowed.svg"_s;
        QTRY_VERIFY_WITH_TIMEOUT(QFile::exists(repaired), 10'000);

        // Outside the square and its shadow, the repaired icon is see-through, not black.
        const QImage image = render(repaired);
        QCOMPARE(qAlpha(image.pixel(60, 4)), 0);
        // The shadow is faint.
        QVERIFY(qAlpha(image.pixel(50, 50)) < 64);
        // The square is still there.
        QCOMPARE(QColor(image.pixel(12, 12)).name(), u"#2ec27e"_s);
    }
};

QTEST_MAIN(TestIconRepair)
#include "tst_iconrepair.moc"

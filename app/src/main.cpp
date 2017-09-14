#include "qquickitemmapboxgl.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qputenv("MAPBOX_ACCESS_TOKEN", "pk.eyJ1IjoicmluaWd1cyIsImEiOiJjajIxZjVseXEwMDBiMzNzYXZrcWo3c2F1In0.DmTyKA-5GcFaCyhlhbCFRA");

    qmlRegisterType<QQuickItemMapboxGL>("QQuickItemMapboxGL", 1, 0, "MapboxMap");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    return app.exec();
}

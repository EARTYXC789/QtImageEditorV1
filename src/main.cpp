#include "mainwindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QIcon>
#include <QLocale>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("QtImageEditorV1"));
    QCoreApplication::setOrganizationName(QStringLiteral("ImageEditorCourse"));
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/editor.png")));

    MainWindow window;
    window.show();
    return app.exec();
}

#include "mainwindow.h"
#include <QApplication>
#include <QDir>
#include <QCommandLineParser>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qSetMessagePattern("[%{type}] %{function}() :\t%{message}");


//    QCoreApplication::setApplicationName("Wargame-LTO");
    QCoreApplication::setApplicationVersion("0.2");

    QCommandLineParser parser;
    parser.setApplicationDescription("Générateur d'ODB (Ordre de Bataille) - LTO"); // HACK 2024
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption nameOption({"n", "name"}, "The name of the session.", "name", "defaultName");
    parser.addOption(nameOption);
    QCommandLineOption adminOption({"a", "admin"}, "Is administrator.");
    parser.addOption(adminOption);
    QCommandLineOption observerOption({"o", "observer"}, "Is observer.");
    parser.addOption(observerOption);

    // Process the actual command line arguments given by the user
    parser.process(a);

    QString sessionName = parser.value(nameOption);
    bool isAdmin = parser.isSet(adminOption);
    bool isObs = parser.isSet(observerOption);
    isAdmin = true; // HACK 2024

    const QStringList args = parser.positionalArguments();
    qDebug() << "Name " << sessionName;
    qDebug() << "Admin " << isAdmin;
    qDebug() << "Obs " << isObs;

    QCoreApplication::setApplicationName(QString("Wargame-LTO - ") + sessionName + (isObs ? " Observer" : ""));
    QCoreApplication::setApplicationName(QString("Générateur d'ODB (Ordre de Bataille) - LTO")); // HACK 2024

    MainWindow mainWindow{isAdmin, sessionName, isObs};
    //    w.openFile(QDir::currentPath() + "/default.json");
    mainWindow.show();
    mainWindow.setSessionName(sessionName);

    return a.exec();
}

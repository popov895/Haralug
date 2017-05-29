#include <QApplication>
#include <QDir>
#include <QLockFile>

#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    application.setApplicationName("Haralug");
    application.setOrganizationName("popov895");

    QLockFile lockFile(QString("%1/%2").arg(QDir::tempPath()).arg(application.applicationName()));
    if (!lockFile.tryLock())
        return 0;

    MainWindow window;
    window.show();

    return application.exec();
}

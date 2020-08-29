#include <QCoreApplication>
#include <QDebug>

#include <stdio.h>

#include "HeliOS_Arduino.h"

// Platform Windows/Linux is selected on .pro

void task(qint16 id)
{
    qDebug() << "task()" << QString::number(id) << " ran at " << CurrentTime() << " us";
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    xHeliOSSetup();

    qint16 id = xTaskAdd("TASK", &task);
    xTaskWait(id);
    xTaskSetTimer(id, 1000000);

    while(1)
    {
        xHeliOSLoop();
    }

    return a.exec();
}

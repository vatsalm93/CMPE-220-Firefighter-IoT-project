#include "audioRecorder.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AudioRecorder w;
    w.show();

    return a.exec();
}

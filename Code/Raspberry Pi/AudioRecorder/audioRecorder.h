#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <QMainWindow>
#include "sender.h"
#include "network.h"

namespace Ui {
class AudioRecorder;
}

class AudioRecorder : public QMainWindow
{
    Q_OBJECT

public:
    explicit AudioRecorder(QWidget *parent = nullptr);
    ~AudioRecorder();

private slots:
    void on_horizontalSlider_valueChanged(int value);
    void on_pBPushToTalk_pressed();
    void on_pBPushToTalk_released();

signals:
    void start_recording(void);
    void stop_recording(void);

private:
    Ui::AudioRecorder *ui;
    int ApplyVolumeToSample(short iSample);
    void initialize_variable(void);
    Sender *audio;
    Network *network;
    int m_iVolume;

};

#endif // AUDIORECORDER_H

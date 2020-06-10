#ifndef SENDER_H
#define SENDER_H

#include <QObject>
#include <QAudioInput>
#include <QAudioOutput>
#include <microphone.h>
#include <speaker.h>

class Sender : public QObject
{
    Q_OBJECT
public:
    explicit Sender(QObject *parent = nullptr);

signals:
    void chunk_dataOutput(QByteArray chunk);
    void not_receiving_state();
    void receiving_state();

public slots:
    void recButtonPressed();
    void recButtonReleased();
    void playAudioPacket(QByteArray audio_packet);

private slots:
    void output_state(QAudio::State state);

private:
    QAudioInput* m_audio_input;
    QAudioOutput* m_audio_output;
    Microphone* m_microphone;
    Speaker* speaker;
};

#endif // SENDER_H

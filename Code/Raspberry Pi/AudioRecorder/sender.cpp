#include "sender.h"
#include <QDebug>

Sender::Sender(QObject *parent) :
    QObject(parent)
{
    QAudioFormat format;
    format.setSampleRate(8000);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);
    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();

    if (!info.isFormatSupported(format)) {
        qWarning() << "Default format not suported. Using nearest format";
        format = info.nearestFormat(format);
    }

    m_audio_input = new QAudioInput(format, this);
    m_audio_output = new QAudioOutput(format, this);

    connect(m_audio_output, SIGNAL(stateChanged(QAudio::State)), this, SLOT(output_state(QAudio::State)));

    // Create Mic
    m_microphone = new Microphone(this);
    m_microphone->open(QIODevice::ReadWrite);

    //this signal is emitted by the mic toward the network module
    //it contains the chunk audio data and the lenght of the chunk
    connect(m_microphone,SIGNAL(chunkDataOutput(QByteArray)),this,SIGNAL(chunk_dataOutput(QByteArray)));

    //create speaker
    speaker = new Speaker(this);
    speaker->open(QIODevice::ReadWrite);
}

//this slot is triggered by a signal emitted in the GUI when
//the user press the rec button
//it emits a signal to the network module that will stop the
//reception of packets
/**
 * @brief Sender::recButtonPressed
 */
void Sender::recButtonPressed()
{
    if (m_audio_input->state() != QAudio::StoppedState) {
        qDebug() << "Unexpected audio input state";
        return;
    }
    m_audio_input->start(m_microphone);
}

//this slot is triggered by a signal emitted in the GUI when
//the user release the rec button
//it emits a signal to the network module that will restart the
//reception of packets
/**
 * @brief Sender::recButtonReleased
 */
void Sender::recButtonReleased()
{
    m_audio_input->stop();
}

//this slot is triggered by a signal emitted in the network module
//every time an audio packet is received
/**
 * @brief Sender::playAudioPacket
 * @param audio_packet
 */
void Sender::playAudioPacket(QByteArray audio_packet)
{
    if(m_audio_output->state() == QAudio::StoppedState)
    {
        m_audio_output->start(speaker);
        qDebug() << "SND_FAC: turning on speaker.";
    }
    speaker->writeData(audio_packet.data(),audio_packet.size());
}

/**
 * @brief Sender::output_state
 * @param state
 */
void Sender::output_state(QAudio::State state)
{
    if(m_audio_output->error() != QAudio::NoError)
    {
        qDebug() << "Output Audio error" << m_audio_output->error();
    }

    if(state == QAudio::IdleState)
    {
        qDebug() << "Stopping output";

        // No more data to play
        speaker->clear_buffer();
        m_audio_output->stop();
    }
}

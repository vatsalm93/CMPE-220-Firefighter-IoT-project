#include "audioRecorder.h"
#include "ui_audioRecorder.h"
#include <QDebug>

AudioRecorder::AudioRecorder(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AudioRecorder)
{
    ui->setupUi(this);
    initialize_variable();
}

AudioRecorder::~AudioRecorder()
{
    delete ui;
}

/**
 * @brief AudioRecorder::initialize_variable
 */
void AudioRecorder::initialize_variable()
{
    audio = new Sender(this);
    connect(this, SIGNAL(start_recording()), audio, SLOT(recButtonPressed()));
    connect(this, SIGNAL(stop_recording()), audio, SLOT(recButtonReleased()));

    network = new Network(this);
    connect(this, SIGNAL(start_recording()), network, SLOT(recButton_pressed()));
    connect(this, SIGNAL(stop_recording()), network, SLOT(recButton_released()));

    connect(audio,SIGNAL(chunk_dataOutput(QByteArray)),network,SLOT(chunkAudioOutput_ready(QByteArray)));
    connect(network,SIGNAL(audioPacket(QByteArray)),audio,SLOT(playAudioPacket(QByteArray)));
}

/**
 * @brief AudioRecorder::ApplyVolumeToSample
 * @param iSample
 * @return
 */
int AudioRecorder::ApplyVolumeToSample(short iSample)
{
    //Calculate volume, Volume limited to  max 35535 and min -35535
    return std::max(std::min(((iSample * m_iVolume) / 50) ,35535), -35535);
}

/**
 * @brief AudioRecorder::on_horizontalSlider_valueChanged
 * @param value
 */
void AudioRecorder::on_horizontalSlider_valueChanged(int value)
{
    m_iVolume = value;
}

/**
 * @brief AudioRecorder::on_pBPushToTalk_pressed
 */
void AudioRecorder::on_pBPushToTalk_pressed()
{
    emit start_recording();
//    //Audio input device
//    m_input = m_audioInput->start();
//    m_input->open(QIODevice::ReadWrite);

//    //connect readyRead signal to readMre slot.
//    //Call readmore when audio samples fill in inputbuffer
//    connect(m_input, SIGNAL(readyRead()), SLOT(readMore()));
//    //Audio output device
//    //write modified sond sample to outputdevice for playback audio
//    m_output = m_audioOutput->start();
//    m_output->open(QIODevice::ReadWrite);
}

/**
 * @brief AudioRecorder::on_pBPushToTalk_released
 */
void AudioRecorder::on_pBPushToTalk_released()
{
    emit stop_recording();
//    m_audioInput->stop();
//    m_audioOutput->stop();
}

//void AudioRecorder::readMore()
//{
//    //Return if audio input is null
//    if(!m_audioInput)
//        return;

//    //Check the number of samples in input buffer
//    qint64 len = m_audioInput->bytesReady();

//    //Limit sample size
//    if(len > 4096)
//        len = 4096;
//    //Read sound samples from input device to buffer
//    qint64 l = m_input->read(m_buffer.data(), len);
//    if(l > 0)
//    {
//        //Assign sound samples to short array
//        short* resultingData = reinterpret_cast<short *>(m_buffer.data());
//        short *outdata=resultingData;
//        outdata[ 0 ] = resultingData [ 0 ];

//        int iIndex;
//        if(ui->chkRemoveNoise->checkState() == Qt::Checked)
//        {
//            //Remove noise using Low Pass filter algortm[Simple algorithm used to remove noise]
//            for ( iIndex=1; iIndex < len; iIndex++ )
//            {
//                outdata[ iIndex ] = 0.333 * resultingData[iIndex ] + ( 1.0 - 0.333 ) * outdata[ iIndex-1 ];
//            }
//        }

//        for ( iIndex=0; iIndex < len; iIndex++ )
//        {
//            //Cange volume to each integer data in a sample
//            outdata[ iIndex ] = ApplyVolumeToSample( outdata[ iIndex ]);
//        }

//        //write modified sond sample to outputdevice for playback audio
//        m_output->write(reinterpret_cast<char *>(outdata), len);
//    }
//}

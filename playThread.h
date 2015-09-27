#ifndef PLAYTHREAD_H
#define PLAYTHREAD_H
#include <QtCore/QThread>
#include <glib.h>
#include <gst/gst.h>
#include <gst/gstbus.h>

#include <QTimer>
#include <QTimerEvent>

#include <QDebug>

class playThread : public QThread
{
    Q_OBJECT

public:
    playThread();
    GstBus *bus;
    GstMessage *msg;
    GstElement *pipeline, *video_sink, *audio_sink, *demux, *audio_queue, *video_queue;
    GstElement *source, *video_dec, *audio_dec, *volume;

    int counter;
    int m_nTimerId;

    void stop(void);
    void play_once(void);
    bool set_playing(void);
    void clean_up(void);
    static void pad_added_handler(GstElement *src, GstPad *new_pad, gpointer user_data);

private slots:

private:
    void timerEvent( QTimerEvent *event );
    void run();
};


#endif // PLAYTHREAD_H

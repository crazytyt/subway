
#include "playThread.h"

static GstPad *queue_audio_pad, *queue_video_pad;
const gchar *UDP_ADDR = "192.168.1.255";
const gint UDP_PORT = 1234;
gboolean is_empty = false;
gboolean audio_connect = false;
gboolean video_connect = false;

#define TS_CAPS "video/mpegts, systemstream=(boolean)true, packetsize=(int)188"

playThread::playThread()
{
    //moveToThread(this);
    gst_init(NULL, NULL);

    m_nTimerId = startTimer(1000);
    counter = 0;
}

enum  {
    overlay_top = 10,
    overlay_left = 330,
    overlay_width = 930,
    overlay_height = 520
};

void playThread::pad_added_handler (GstElement *src, GstPad *new_pad, gpointer user_data)
{
    GstCaps *new_pad_caps = NULL;
    GstStructure *new_pad_struct = NULL;
    const gchar *new_pad_type = NULL;
    user_data = user_data;
    src = src;

    GstPadLinkReturn ret;

    /* Check the new pad's type */
    new_pad_caps = gst_pad_get_current_caps(new_pad);
    new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
    new_pad_type = gst_structure_get_name (new_pad_struct);

    if (g_str_has_prefix (new_pad_type, "audio")) {
        /* If queue_audio_pad already linked, we have nothing to do here */
        if (gst_pad_is_linked (queue_audio_pad)) {
            g_print ("  We are already linked. Ignoring.\n");
            return;
        }
        ret = gst_pad_link(new_pad, queue_audio_pad);
        if (GST_PAD_LINK_FAILED (ret)) {
            g_print ("  Type is '%s' but link failed.\n", new_pad_type);
            audio_connect = false;
        } else
            audio_connect = true;
    }

    if (g_str_has_prefix (new_pad_type, "video")) {
        /* If queue_video_pad already linked, we have nothing to do here */
        if (gst_pad_is_linked (queue_video_pad)) {
            g_print ("  We are already linked. Ignoring.\n");
            return;
        }
        ret = gst_pad_link (new_pad, queue_video_pad);
        if (GST_PAD_LINK_FAILED (ret)) {
            g_print ("  Type is '%s' but link failed.\n", new_pad_type);
            video_connect = false;
        } else
            video_connect = true;
    }
}

void playThread::timerEvent(QTimerEvent *event)
{
    event = event;
    guint64 time;

    if (video_connect != true || audio_connect != true)
        return;

    g_object_get(video_queue, "current-level-time", &time, NULL);
    if (time < 10000)
        counter++;
    else
        counter = 0;
    if (counter > 5)
        is_empty = true;
}

bool playThread::set_playing(void)
{
    GstStateChangeReturn ret;
    GstCaps *caps;

    /* init all the element */
    pipeline = gst_pipeline_new ("my_pipeline");
    source = gst_element_factory_make("udpsrc", "source");
    audio_queue = gst_element_factory_make("queue", "audio_queue");
    video_queue = gst_element_factory_make("queue", "video_queue");
    demux = gst_element_factory_make("aiurdemux", "demux");
    audio_dec = gst_element_factory_make("beepdec", "audio_dec");
    video_dec = gst_element_factory_make("vpudec", "video_dec");
    volume = gst_element_factory_make("volume", "volume");
    audio_sink = gst_element_factory_make("alsasink", "audio_sink");
    video_sink = gst_element_factory_make("imxv4l2sink", "video_sink");
    if (!pipeline || !source || !audio_queue || !video_queue || !demux ||
            !audio_dec || !video_dec || !volume || !audio_sink || !video_sink) {
        g_printerr ("Not all elements could created.\n");
        return false;
    }

    /* set udpsrc parameter */
    g_object_set(source, "multicast-group", UDP_ADDR, "port", UDP_PORT, NULL);
    caps = gst_caps_from_string(TS_CAPS);
    g_object_set(source, "caps", caps, NULL);

    /* set volume */
    g_object_set(volume, "volume", 1.0, NULL);

    /* set video decode */
    g_object_set(video_dec, "frame-drop", false, NULL);

    /* set the display area */
    g_object_set(video_sink, "overlay-top", overlay_top, "overlay-left", overlay_left,
                 "overlay-width", overlay_width, "overlay-height", overlay_height, NULL);

    //g_object_set(video_queue, "max-size-buffers", 0, "max-size-time", 0, NULL);
    //g_object_set(audio_queue, "max-size-buffers", 0, "max-size-time", 0, NULL);

    g_object_set(demux, "streaming-latency", 500, NULL);

    /* link all the element except the demux */
    gst_bin_add_many (GST_BIN (pipeline), source, audio_queue, video_queue, demux, audio_dec, video_dec,
                      volume, audio_sink, video_sink, NULL);

    if (gst_element_link_many (source, demux, NULL) != TRUE ||
            gst_element_link_many (audio_queue, audio_dec, volume, audio_sink, NULL) != TRUE ||
            gst_element_link_many (video_queue, video_dec, video_sink, NULL) != TRUE) {
        g_printerr ("Elements could not be linked.\n");
        return false;
    }

    /* initialize the audio and video pad, will link it in pad_added_handler */
    queue_audio_pad = gst_element_get_static_pad (audio_queue, "sink");
    queue_video_pad = gst_element_get_static_pad (video_queue, "sink");

    g_signal_connect (demux, "pad-added", G_CALLBACK (pad_added_handler), NULL);
    //g_signal_connect(video_queue, "underrun", G_CALLBACK(video_empty_handler), pipeline);

    bus = gst_element_get_bus (pipeline);

    ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        gst_object_unref (bus);
        return false;

    }
    return true;
}


void playThread::clean_up()
{
    is_empty = false;
    audio_connect = video_connect = false;
    counter = 0;

    /* Free resources */
    if (msg != NULL)
        gst_message_unref (msg);
    if (!bus)
        gst_object_unref (bus);
    gst_element_set_state (pipeline, GST_STATE_NULL);
    if (!pipeline)
        gst_object_unref (pipeline);
    if (!source)
        gst_object_unref (source);
    if (!video_queue)
        gst_object_unref (video_queue);
    if (!audio_queue)
        gst_object_unref (audio_queue);
    if (!video_sink)
        gst_object_unref (video_sink);
    if (!audio_sink)
        gst_object_unref (audio_sink);
    if (!volume)
        gst_object_unref (volume);
    if (!video_dec)
        gst_object_unref (video_dec);
    if (!audio_dec)
        gst_object_unref (audio_dec);
    if (!demux)
        gst_object_unref (demux);
    if (!queue_audio_pad)
        gst_object_unref (queue_audio_pad);
    if (!queue_video_pad)
        gst_object_unref (queue_video_pad);
}

void playThread::play_once()
{

    do {
        if (set_playing() == true)
            break;
        else {

            clean_up();
            QThread::sleep(4);
        }
    } while (1);

    do {
        msg = gst_bus_timed_pop_filtered (bus, GST_SECOND * 2, GST_MESSAGE_ERROR);

        /* timeout process */
        if (msg != NULL || is_empty == true)
            break;

    } while(1);

}

void playThread::run()
{

    do {

        play_once();
        clean_up();

        QThread::sleep(2);
        g_print(" T: start again. ");
    } while(1);
}

void playThread::stop(void)
{

    clean_up();
    qDebug() << " stop ... ";
}

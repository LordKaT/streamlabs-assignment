#include "assignment.h"

// callback function for bus messages
static void busMessageCallback(GstBus *bus, GstMessage *msg, ContainerData *data) {
  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR: {
      GError *err;
      gchar *debug;
      gst_message_parse_error(msg, &err, &debug);
      g_print("Err: %s\r\n", err->message);
      g_error_free(err);
      g_free(debug);
      gst_element_set_state(data->pipeline, GST_STATE_READY);
      g_main_loop_quit(data->loop);
      break;
    }
    case GST_MESSAGE_EOS:
      g_print("End of stream.\r\n");
      break;
    default:
      break;
  }
}

// handler for keyboard input
static gboolean keyboardHandler(GIOChannel *source, GIOCondition cond, ContainerData *data) {
  gchar *str = NULL;

  if (g_io_channel_read_line(source, &str, NULL, NULL, NULL) != G_IO_STATUS_NORMAL)
    return TRUE;

  switch (g_ascii_tolower(str[0])) {
    case 'x': { // change layout
      if (data->layout == 1) {
        g_object_set(data->sink_0, "xpos", 320, "ypos", 0, NULL);
        g_object_set(data->sink_1, "xpos", 320, "ypos", 240, NULL);
        g_object_set(data->sink_2, "xpos", 0, "ypos", 160, NULL);
      } else if (data->layout == 2) {
        g_object_set(data->sink_0, "xpos", 0, "ypos", 240, NULL);
        g_object_set(data->sink_1, "xpos", 320, "ypos", 240, NULL);
        g_object_set(data->sink_2, "xpos", 160, "ypos", 0, NULL);
      } else if (data->layout == 3) {
        g_object_set(data->sink_0, "xpos", 0, "ypos", 0, NULL);
        g_object_set(data->sink_1, "xpos", 0, "ypos", 240, NULL);
        g_object_set(data->sink_2, "xpos", 320, "ypos", 160, NULL);
      } else if (data->layout == 4) {
        g_object_set(data->sink_0, "xpos", 0, "ypos", 0, NULL);
        g_object_set(data->sink_1, "xpos", 320, "ypos", 0, NULL);
        g_object_set(data->sink_2, "xpos", 160, "ypos", 240, NULL);
      }
      data->layout++;
      if (data->layout == 5)
        data->layout = 1;
      break;
    }
    case 'q': // quit
      g_main_loop_quit(data->loop);
      break;
    default:
      break;
  }

  g_free(str);
  return TRUE;
}

// memcpy is faster than strcat by a lot, just don't screw up your maths.
char *fastconcat(const char *string1, const char *string2) {
  const size_t len1 = strlen(string1);
  const size_t len2 = strlen(string2);
  gchar *res = malloc(len1 + len2 + 1);
  memcpy(res, string1, len1);
  memcpy(res + len1, string2, len2 + 1);
  return res;
}

int main (int argc, char *argv[]) {
  ContainerData data;
  GstStateChangeReturn ret;
  GIOChannel *io_stdin;
  // I'm using a funky 960x540 resolution to save my VM some headaches in rendering.
  gchar *pipelineCommandTemplate =
      "videomixer name=mix sink_0::xpos=0 sink_1::xpos=320 sink_2::xpos=160 sink_2::ypos=240 background=black ! videoscale ! video/x-raw,width=960,height=540 ! tee name=t "
      "filesrc name=file1 location=./sample.mp4 ! decodebin ! videoconvert ! videoscale ! video/x-raw,width=320,height=240 ! mix.sink_0 "
      "filesrc name=file2 location=./sample.mp4 ! decodebin ! videoconvert ! videoscale ! video/x-raw,width=320,height=240 ! videoflip method=rotate-180 ! mix.sink_1 "
      "filesrc name=file3 location=./sample.mp4 ! decodebin ! videoconvert ! videoscale ! video/x-raw,width=320,height=240 ! coloreffects preset=xray ! mix.sink_2 "
      "t. ! queue ! videoconvert ! autovideosink "
      "t. ! queue ! videoconvert ! x264enc tune=zerolatency bitrate=800 byte-stream=false key-int-max=60 bframes=0 aud=true ! flvmux streamable=true ! rtmpsink name=rtmp_twitch location=";
  gchar *pipelineCommand;
  gchar *ingest;

  IngestServer ingestServers[3] = {
      {"1", "rtmp://jfk.contribute.live-video.net/app/"},
      {"2", "rtmp://dfw.contribute.live-video.net/app/"},
      {"3", "rtmp://pdx.contribute.live-video.net/app/"}
  };

  gst_init(&argc, &argv);

  if (argc <= 2) {
    g_print("Usage: assignment <ingest server> <stream id>\r\n"
        " Ingest server can be one of the following:\r\n"
        "  1 - East Coast (NY)\r\n"
        "  2 - Central (Dallas)\r\n"
        "  3 - West Coast (Portland)\r\n");
    return 0;
  }

  gint i = 0;
  while (i < 3) {
    if (!strcmp(argv[1], ingestServers[i].id)) // found ingest server
      break;
    i++;
  }

  if (i == 3) {
    g_print("invalid ingest server.\r\n");
    return 0;
  }

  ingest = fastconcat(ingestServers[i].name, argv[2]);
  pipelineCommand = fastconcat(pipelineCommandTemplate, ingest);

  memset(&data, 0, sizeof(data));
  data.layout = 1;

  g_print("Use: type your option in the console and press enter:\r\n"
      " 'X' - change video layout\r\n"
      " 'Q' - quit program\r\n");

  // builds the pipeline
  data.pipeline = gst_parse_launch(pipelineCommand, NULL);

#ifdef G_OS_WIN32
  io_stdin = g_io_channel_win32_new_fd(fileno(stdin));
#else
  io_stdin = g_io_channel_unix_new(fileno(stdin));
#endif
  g_io_add_watch(io_stdin, G_IO_IN, (GIOFunc) keyboardHandler, &data);

  // attach to the pipeline bus and intercept messages
  data.bus = gst_element_get_bus(data.pipeline);
  gst_bus_add_signal_watch(data.bus);
  g_signal_connect(data.bus, "message", G_CALLBACK(busMessageCallback), &data);

  // This actually starts pushing video out
  ret = gst_element_set_state(data.pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr("Could not set pipeline to playing state.\r\n");
    gst_object_unref(data.pipeline);
    return -1;
  }

  g_print("Now playing\r\n");

  data.playing = TRUE;

  // Grab the videoMixer and all the sink_'s we need to manipulate later
  data.videoMixer = gst_bin_get_by_name(GST_BIN(data.pipeline), "mix");
  data.sink_0 = gst_element_get_static_pad(data.videoMixer, "sink_0");
  data.sink_1 = gst_element_get_static_pad(data.videoMixer, "sink_1");
  data.sink_2 = gst_element_get_static_pad(data.videoMixer, "sink_2");

  data.loop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(data.loop);

  g_main_loop_unref(data.loop);
  gst_element_set_state(data.pipeline, GST_STATE_NULL);
  gst_object_unref(data.pipeline);

  return 0;
}

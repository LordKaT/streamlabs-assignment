#ifndef __ASSIGNMENT_H__
#define __ASSIGNMENT_H__

#include <stdio.h>
#include <stdint.h>
#include <gst/gst.h>
#include <glib-object.h>

typedef struct _ContainerData {
  GstElement *pipeline;
  GstElement *videoMixer;
  GstPad *sink_0, *sink_1, *sink_2; // three videos, three sinks
  GstBus *bus;
  GstMessage *msg;
  GMainLoop *loop;
  gboolean playing;
  //gchar *launchCommand;
  gchar layout;
} ContainerData;

typedef struct _SinkData {
  gint *xpos;
  gint *ypos;
} SinkData;

typedef struct _IngestServer {
  gchar *id;
  gchar *name;
} IngestServer;

#endif

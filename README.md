Reqs: gcc, gstreamer-1.0, associated dev libs

Compile: gcc assignment.c -o assignment $(pkg-config --cflags --libs gstreamer-1.0)

Run: ./assignment <ingest option> <twitch stream id>\
 Where <ingest option> is one of the following:\
  1 - NY Ingest\
  2 - Dallas Ingest\
  3 - Portland Ingest\

To enter commands while streaming type in the letter of the command followed by enter\
in the console.

**Comments**\
Thank you for the opportunity to take on this interesting challenge. I have\
spent the majority of my 8 hours researching as C and gstreamer API are\
outside my experience. Given that, I'm happy with how this turned out. There\
are some areas I want to improve on, especially using gst_parse_launch, I\
*almost* came up with a solution using gst_element_factory_make but got hung\
up on linking videomixer and the three video files progmatically, so I fell\
back to using gst_parse_launch to get this running in the time alloted.

There's much that can be improved here but I got to learn the basics of a\
super useful API that I've never had the chance to use before. That's a\
win in my book.

Thanks again for considering me. Good luck with your search! Rock on.

Jason

===
mts
===

mts is a library for parsing mpegts files and streams.

.. contents:: Table of Contents:
   :local:

Usage
=====

For an example of how to use this library please look in the example folder.

Inspection
----------
For an example of how to use mts for inspecting the content of a mpegts file
please see ``examples/mpegts_inspect.cpp``.

You can test this example like so::

    python waf configure
    > ...
    python waf build
    > ...
    python waf install --install_path ./bin
    > ...
    ./bin/mpegts_inspect test/test.ts
    > ISO/IEC 13818-7 Audio with ADTS transport syntax: 33
    > AVC video stream: 165

H.264 Extraction
----------------
For an example of how to use mts for extracting H.264 data from a mpegts file
please see ``examples/mpegts_to_h264.cpp``.

You can test this example like so::

    python waf configure
    > ...
    python waf build
    > ...
    python waf install --install_path ./bin
    > ...
    ./bin/mpegts_to_h264 test/test.ts out.h264

Playback the extracted h264 data with vlc like so::

    cvlc out.h264 -v --no-loop --play-and-exit

AAC Extraction
--------------
For an example of how to use mts for extracting aac data from a mpegts file
please see ``examples/mpegts_to_aac.cpp``.

You can test this example like so::

    python waf configure
    > ...
    python waf build
    > ...
    python waf install --install_path ./bin
    > ...
    ./bin/mpegts_to_aac test/test.ts out.aac

Playback the extracted aac data with vlc like so::

    cvlc out.aac -v --no-loop --play-and-exit

// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

STREAM_TYPE_TAG(
    0x00,
    reserved,
    "Reserved")

STREAM_TYPE_TAG(
    0x01,
    video_11172_2,
    "ISO/IEC 11172-2 Video")

STREAM_TYPE_TAG(
    0x02,
    video_13818_2,
    "ISO/IEC 13818-2 Video")

STREAM_TYPE_TAG(
    0x03,
    audio_11172_3,
    "ISO/IEC 11172-3 Audio")

STREAM_TYPE_TAG(
    0x04,
    audio_13818_3,
    "ISO/IEC 13818-3 Audio")

STREAM_TYPE_TAG(
    0x05,
    private_section_13818_1,
    "ISO/IEC 13818-1 private_sections")

STREAM_TYPE_TAG(
    0x06,
    private_data_d13818_1,
    "ISO/IEC 13818-1 PES packets containing private data")

STREAM_TYPE_TAG(
    0x07,
    mheg13522,
    "ISO/IEC 13522 MHEG")

STREAM_TYPE_TAG(
    0x08,
    annex_a_13818_1,
    "ISO/IEC 13818-1 Annex A DSM-CC")

STREAM_TYPE_TAG(
    0x09,
    h_222_1,
    "Rec. ITU-T H.222.1")

STREAM_TYPE_TAG(
    0x0A,
    type_a_13818_6,
    "ISO/IEC 13818-6 type A")

STREAM_TYPE_TAG(
    0x0B,
    type_b_13818_6,
    "ISO/IEC 13818-6 type B")

STREAM_TYPE_TAG(
    0x0C,
    type_c_13818_6,
    "ISO/IEC 13818-6 type C")

STREAM_TYPE_TAG(
    0x0D,
    type_d_13818_6,
    "ISO/IEC 13818-6 type D")

STREAM_TYPE_TAG(
    0x0E,
    auxiliary_13818_1,
    "ISO/IEC 13818-1 auxiliary")

STREAM_TYPE_TAG(
    0x0F,
    adts_transport_13818_7,
    "ISO/IEC 13818-7 Audio with ADTS transport syntax")

STREAM_TYPE_TAG(
    0x10,
    visual_14496_2,
    "ISO/IEC 14496-2 Visual")

STREAM_TYPE_TAG(
    0x11,
    latm_transport_14496_3,
    "ISO/IEC 14496-3 Audio with the LATM transport syntax")

STREAM_TYPE_TAG(
    0x12,
    stream_type12,
    "SL-packetized or FlexMux stream in PES packets")

STREAM_TYPE_TAG(
    0x13,
    stream_type13,
    "SL-packetized or FlexMux stream in ISO/IEC 14496 sections")

STREAM_TYPE_TAG(
    0x14,
    stream_type14,
    "ISO/IEC 13818-6 Synchronized Download Protocol")

STREAM_TYPE_TAG(
    0x15,
    metadata15,
    "Metadata carried in PES packets")

STREAM_TYPE_TAG(
    0x16,
    metadata16,
    "Metadata carried in metadata_sections")

STREAM_TYPE_TAG(
    0x17,
    metadata17,
    "Metadata carried in ISO/IEC 13818-6 Data Carousel")

STREAM_TYPE_TAG(
    0x18,
    metadata18,
    "Metadata carried in ISO/IEC 13818-6 Object Carousel")

STREAM_TYPE_TAG(
    0x19,
    metadata19,
    "Metadata carried in ISO/IEC 13818-6 Synchronized Download Protocol")

STREAM_TYPE_TAG(
    0x1A,
    mpeg_2_ipmp_stream,
    "MPEG-2 IPMP stream")

STREAM_TYPE_TAG(
    0x1B,
    avc_video_stream,
    "AVC video stream")

STREAM_TYPE_TAG(
    0x1C,
    no_transport_14496_3,
    "ISO/IEC 14496-3 Audio, without using any additional transport syntax")

STREAM_TYPE_TAG(
    0x1D,
    text_14496_17,
    "ISO/IEC 14496-17 Text")

STREAM_TYPE_TAG(
    0x1E,
    auxiliary_video_stream,
    "Auxiliary video stream as defined in ISO/IEC 23002-3")

STREAM_TYPE_TAG(
    0x1F,
    svc_video_sub_bitstream,
    "SVC video sub-bitstream of an AVC video stream")

STREAM_TYPE_TAG(
    0x20,
    mvc_video_sub_bitstream,
    "MVC video sub-bitstream of an AVC video stream")

STREAM_TYPE_TAG(
    0x21,
    stream_type21,
    "Video stream conforming to one or more profiles")

STREAM_TYPE_TAG(
    0x22,
    stream_type22,
    "Bideo stream for service-compatible stereoscopic 3D services")

STREAM_TYPE_TAG(
    0x23,
    stream_type23,
    "ISO/IEC 14496-10 video stream conforming to one or more profiles")

STREAM_TYPE_TAG(
    0x24,
    first_13818_1_reserved,
    "First ISO/IEC 13818-1 Reserved")

STREAM_TYPE_TAG(
    0x7E,
    last_13818_1_reserved,
    "Last ISO/IEC 13818-1 Reserved")

STREAM_TYPE_TAG(
    0x7F,
    ipmp_stream,
    "IPMP stream")

STREAM_TYPE_TAG(
    0x80,
    first_user_private,
    "First User Private")

STREAM_TYPE_TAG(
    0xFF,
    last_user_private,
    "Last User Private")


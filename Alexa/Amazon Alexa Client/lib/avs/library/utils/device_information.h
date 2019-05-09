#ifndef FT900_DEVICE_INFORMATION_H_
#define FT900_DEVICE_INFORMATION_H_



/************************************************/
/* DEVICE CAPABILITIES                          */
/************************************************/
/*BITS  3-0:  format
              0000-raw
              0001-mp3
              0010-wav
              0011-aac
*************************************************/
/*BITS  7-4:  bitDepth
              0000-8  bits 
              0001-16 bits 
              0010-24 bits
              0011-32 bits  
*************************************************/
/*BITS 11-8:  bitRate
              0000-16000 hz
              0001-32000 hz
              0010-44100 hz
              0011-48000 hz
              0100-96000 hz
              0101-8000  hz
*************************************************/
/*BITS 15-12: channels
              0000-1 ch mono
              0001-2 ch stereo
*************************************************/


#define DEVICE_CAPABILITIES_OFFSET_FORMAT    0
#define DEVICE_CAPABILITIES_OFFSET_BITDEPTH  4
#define DEVICE_CAPABILITIES_OFFSET_BITRATE   8
#define DEVICE_CAPABILITIES_OFFSET_CHANNEL   12

#define DEVICE_CAPABILITIES_MASK_FORMAT      0xF
#define DEVICE_CAPABILITIES_MASK_BITDEPTH    0xF
#define DEVICE_CAPABILITIES_MASK_BITRATE     0xF
#define DEVICE_CAPABILITIES_MASK_CHANNEL     0xF

#define DEVICE_CAPABILITIES_FORMAT_RAW       0
#define DEVICE_CAPABILITIES_FORMAT_MP3       1
#define DEVICE_CAPABILITIES_FORMAT_WAV       2
#define DEVICE_CAPABILITIES_FORMAT_AAC       3

#define DEVICE_CAPABILITIES_BITDEPTH_8       0
#define DEVICE_CAPABILITIES_BITDEPTH_16      1
#define DEVICE_CAPABILITIES_BITDEPTH_24      2
#define DEVICE_CAPABILITIES_BITDEPTH_32      3

#define DEVICE_CAPABILITIES_BITRATE_16000    0
#define DEVICE_CAPABILITIES_BITRATE_32000    1
#define DEVICE_CAPABILITIES_BITRATE_44100    2
#define DEVICE_CAPABILITIES_BITRATE_48000    3
#define DEVICE_CAPABILITIES_BITRATE_96000    4
#define DEVICE_CAPABILITIES_BITRATE_8000     5

#define DEVICE_CAPABILITIES_CHANNEL_1        0
#define DEVICE_CAPABILITIES_CHANNEL_2        1


/************************************************/


#define GET_DEVICE_CAPABILITIES_FORMAT(x)   ( (x >> DEVICE_CAPABILITIES_OFFSET_FORMAT)   & DEVICE_CAPABILITIES_MASK_FORMAT   )
#define GET_DEVICE_CAPABILITIES_BITDEPTH(x) ( (x >> DEVICE_CAPABILITIES_OFFSET_BITDEPTH) & DEVICE_CAPABILITIES_MASK_BITDEPTH )
#define GET_DEVICE_CAPABILITIES_BITRATE(x)  ( (x >> DEVICE_CAPABILITIES_OFFSET_BITRATE)  & DEVICE_CAPABILITIES_MASK_BITRATE  )
#define GET_DEVICE_CAPABILITIES_CHANNEL(x)  ( (x >> DEVICE_CAPABILITIES_OFFSET_CHANNEL)  & DEVICE_CAPABILITIES_MASK_CHANNEL  )

#define SET_DEVICE_CAPABILITIES(format, depth, rate, channel) \
    ( ( (format  & DEVICE_CAPABILITIES_MASK_FORMAT)   << DEVICE_CAPABILITIES_OFFSET_FORMAT   ) | \
      ( (depth   & DEVICE_CAPABILITIES_MASK_BITDEPTH) << DEVICE_CAPABILITIES_OFFSET_BITDEPTH ) | \
      ( (rate    & DEVICE_CAPABILITIES_MASK_BITRATE)  << DEVICE_CAPABILITIES_OFFSET_BITRATE  ) | \
      ( (channel & DEVICE_CAPABILITIES_MASK_CHANNEL)  << DEVICE_CAPABILITIES_OFFSET_CHANNEL  ) )


/************************************************/


#pragma pack(push, 1)
typedef struct _TDeviceInfo {

    unsigned int m_ulDeviceID;
    unsigned short m_uwSendCapabilities;
    unsigned short m_uwRecvCapabilities;

} TDeviceInfo;
#pragma pack(pop)


/************************************************/



#endif // FT900_DEVICE_INFORMATION_H_

#ifndef AVS_CONFIG_DEFAULTS_H
#define AVS_CONFIG_DEFAULTS_H


#ifndef AVS_CONFIG_SERVER_ADDR
#error "AVS_CONFIG_SERVER_ADDR must be set"
#endif

#ifndef AVS_CONFIG_SERVER_PORT
#define AVS_CONFIG_SERVER_PORT          (11234)
#endif

#ifndef AVS_CONFIG_SAMPLING_RATE
#define AVS_CONFIG_SAMPLING_RATE        SAMPLING_RATE_16KHZ
#endif

// Ethernet RX FIFO is 4kb so 2kb is fine
#ifndef AVS_CONFIG_RX_SIZE
#define AVS_CONFIG_RX_SIZE              (1460*2)
#endif

// Ethernet TX FIFO is 2kb so just 1 packet
#ifndef AVS_CONFIG_TX_SIZE
#define AVS_CONFIG_TX_SIZE              (1460)
#endif

#ifndef AVS_CONFIG_RXTX_BUFFER_SIZE
#define AVS_CONFIG_RXTX_BUFFER_SIZE     (6144)
#endif

#ifndef AVS_CONFIG_AUDIO_BUFFER_SIZE
#define AVS_CONFIG_AUDIO_BUFFER_SIZE    (2048)
#endif

#ifndef AVS_CONFIG_RX_TIMEOUT
#define AVS_CONFIG_RX_TIMEOUT           (10)
#endif

#ifndef AVS_CONFIG_TX_TIMEOUT
#define AVS_CONFIG_TX_TIMEOUT           (10)
#endif

#ifndef AVS_CONFIG_MAX_RECORD_SIZE
#define AVS_CONFIG_MAX_RECORD_SIZE      (1024000)
#endif


#endif // AVS_CONFIG_DEFAULTS_H

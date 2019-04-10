/**
  @file avs.c
  @brief
  Amazon Alexa Client library.

 */
/*
 * ============================================================================
 * History
 * =======
 * 2019-02-14 : Created v1
 *
 * Copyright (C) Bridgetek Pte Ltd
 * ============================================================================
 *
 * This source code ("the Software") is provided by Bridgetek Pte Ltd
 * ("Bridgetek") subject to the licence terms set out
 * http://brtchip.com/BRTSourceCodeLicenseAgreement/ ("the Licence Terms").
 * You must read the Licence Terms before downloading or using the Software.
 * By installing or using the Software you agree to the Licence Terms. If you
 * do not agree to the Licence Terms then do not download or use the Software.
 *
 * Without prejudice to the Licence Terms, here is a summary of some of the key
 * terms of the Licence Terms (and in the event of any conflict between this
 * summary and the Licence Terms then the text of the Licence Terms will
 * prevail).
 *
 * The Software is provided "as is".
 * There are no warranties (or similar) in relation to the quality of the
 * Software. You use it at your own risk.
 * The Software should not be used in, or for, any medical device, system or
 * appliance. There are exclusions of Bridgetek liability for certain types of loss
 * such as: special loss or damage; incidental loss or damage; indirect or
 * consequential loss or damage; loss of income; loss of business; loss of
 * profits; loss of revenue; loss of contracts; business interruption; loss of
 * the use of money or anticipated savings; loss of information; loss of
 * opportunity; loss of goodwill or reputation; and/or loss of, damage to or
 * corruption of data.
 * There is a monetary cap on Bridgetek's liability.
 * The Software may have subsequently been amended by another user and then
 * distributed by that other user ("Adapted Software").  If so that user may
 * have additional licence terms that apply to those amendments. However, Bridgetek
 * has no liability in relation to those amendments.
 * ============================================================================
 */

#include "ft900.h"
#include "tinyprintf.h"         // tinyprintf 3rd-party library
#include "FreeRTOS.h"           // FreeRTOS 3rd-party library
#include "task.h"               // FreeRTOS 3rd-party library
#include "semphr.h"             // FreeRTOS 3rd-party library

#include "avs/avs.h"            // AVS library
#include "avs_config.h"         // AVS configuration
#include "utils/audio.h"        // Audio utility
#include "utils/sdcard.h"       // SD card utility
#include "utils/comm_wrapper.h" // Communication wrapper



//#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(...) do {tfp_printf(__VA_ARGS__);} while (0)
#else
#define DEBUG_PRINTF(...)
#endif



#define USE_DO_DIR 0
#define USE_STEREO_TO_MONO_AVERAGE 0 // Using discard instead of average is better audio quality.
#define USE_MULTITHREADED_RECVPLAY 0
#define USE_SENDRECV_MUTEX 1
#define USE_RECORDPLAY_MUTEX 0


#if USE_MULTITHREADED_RECVPLAY
typedef struct _ThreadPlayerContext {

    TaskHandle_t m_xTask;
    SemaphoreHandle_t m_xMutexFile;
    SemaphoreHandle_t m_xMutexRun;
    FIL m_fHandle;
    uint32_t m_ulWriteSize;
    uint32_t m_ulReadSize;
    uint32_t m_ulRecvSize;

} ThreadPlayerContext;

static ThreadPlayerContext g_hContext;
static void vPlayerTask(void *pvParameters);
#endif

static char* g_pcAudioBuffer  = NULL;
static char* g_pcSDCardBuffer = NULL;
#if USE_SENDRECV_MUTEX
static SemaphoreHandle_t m_xMutexSendRecv = NULL;
#endif
#if USE_RECORDPLAY_MUTEX
static SemaphoreHandle_t m_xMutexRecordPlay = NULL;
#endif



#ifdef DEBUG
static const char* getConfigSamplingRateStr()
{
    if (AVS_CONFIG_SAMPLING_RATE == SAMPLING_RATE_44100HZ) {
        return "44.1KHz";
    }
    else if (AVS_CONFIG_SAMPLING_RATE == SAMPLING_RATE_48KHZ) {
        return "48KHz";
    }
    else if (AVS_CONFIG_SAMPLING_RATE == SAMPLING_RATE_32KHZ) {
        return "32KHz";
    }
    else if (AVS_CONFIG_SAMPLING_RATE == SAMPLING_RATE_16KHZ) {
        return "16KHz";
    }
    else if (AVS_CONFIG_SAMPLING_RATE == SAMPLING_RATE_8KHZ) {
        return "8KHz";
    }

    return "Unknown";
}
#endif // DEBUG



int avs_init(void)
{
    // Initialize audio
    audio_setup(NULL, AVS_CONFIG_SAMPLING_RATE);
    DEBUG_PRINTF("Audio initialize.\r\n");

    // Initialize SD card
    sdcard_setup();
    DEBUG_PRINTF("SDCard initialize.\r\n");

    // Pre-allocate the buffers now for faster communication
    g_pcSDCardBuffer = pvPortMalloc(AVS_CONFIG_SDCARD_BUFFER_SIZE);
    g_pcAudioBuffer = pvPortMalloc(AVS_CONFIG_AUDIO_BUFFER_SIZE);
    if (!g_pcAudioBuffer || !g_pcSDCardBuffer) {
        DEBUG_PRINTF("avs_init(): pvPortMalloc failed %p %p\n",
            g_pcAudioBuffer, g_pcSDCardBuffer);
        return 0;
    }
    DEBUG_PRINTF("Memory initialize.\r\n");

    // Initialize mutex
#if USE_SENDRECV_MUTEX
    m_xMutexSendRecv = xSemaphoreCreateMutex();
#endif
#if USE_RECORDPLAY_MUTEX
    m_xMutexRecordPlay = xSemaphoreCreateMutex();
#endif

#if USE_MULTITHREADED_RECVPLAY
    g_hContext.m_xMutexFile = xSemaphoreCreateMutex();
    g_hContext.m_xMutexRun = xSemaphoreCreateMutex();
    if (!g_hContext.m_xMutexRun || !g_hContext.m_xMutexFile) {
        DEBUG_PRINTF("avs_init(): xSemaphoreCreateMutex failed\n");
        avs_free();
        return 0;
    }
    xSemaphoreTake(g_hContext.m_xMutexRun, pdMS_TO_TICKS(portMAX_DELAY));

    // Initialize task
    g_hContext.m_xTask = NULL;
    if (xTaskCreate(vPlayerTask, "Player", 1024, &g_hContext, 1, &g_hContext.m_xTask) != pdTRUE) {
        DEBUG_PRINTF("avs_init(): xTaskCreate failed\n");
        avs_free();
        return 0;
    }
#endif

    return 1;
}

void avs_free(void)
{
    if (g_pcAudioBuffer) {
        vPortFree(g_pcAudioBuffer);
        g_pcAudioBuffer = NULL;
    }

    if (g_pcSDCardBuffer) {
        vPortFree(g_pcSDCardBuffer);
        g_pcSDCardBuffer = NULL;
    }

#if USE_MULTITHREADED_RECVPLAY
    if (g_hContext.m_xMutexFile) {
        vSemaphoreDelete(g_hContext.m_xMutexFile);
        g_hContext.m_xMutexFile = NULL;
    }

    if (g_hContext.m_xMutexRun) {
        vSemaphoreDelete(g_hContext.m_xMutexRun);
        g_hContext.m_xMutexRun = NULL;
    }
#endif

#if USE_SENDRECV_MUTEX
    if (m_xMutexSendRecv) {
        vSemaphoreDelete(m_xMutexSendRecv);
        m_xMutexSendRecv = NULL;
    }
#endif

#if USE_RECORDPLAY_MUTEX
    if (m_xMutexRecordPlay) {
        vSemaphoreDelete(m_xMutexRecordPlay);
        m_xMutexRecordPlay = NULL;
    }
#endif
}

int avs_get_server_port(void)
{
    return comm_get_server_port();
}

const void* avs_get_server_addr(void)
{
    return comm_get_server_addr();
}

int avs_err(void)
{
    return comm_err();
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Establishes connection to the RPI Alexa Gateway using configurations in avs_config.h configuration file.
/////////////////////////////////////////////////////////////////////////////////////////////
int avs_connect(void)
{
    return comm_connect();
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Closes connection with RPI Alexa Gateway.
/////////////////////////////////////////////////////////////////////////////////////////////
void avs_disconnect(void)
{
    comm_disconnect();
}

int avs_isconnected(void)
{
    return comm_isconnected();
}



/////////////////////////////////////////////////////////////////////////////////////////////
// Record audio file from microphone and save to SD card given the complete file path
// - Read 2KB from microphone
// - Convert 2KB stereo to 1KB mono
// - Save 1KB to SD card
// Audio recorded: 16-bit PCM, 16KHZ, stereo (2-channels)
// Audio saved:    16-bit PCM, 16KHZ, mono (1-channel)
/////////////////////////////////////////////////////////////////////////////////////////////
int avs_record_request(const char* pcFileName, char (*fxnCallbackRecord)(void))
{
    FIL fHandle;
    uint32_t ulRecordSize = 0;
    uint32_t ulBytesWritten = 0;
    char* pcMicrophone = g_pcAudioBuffer;


    audio_mic_begin();

    // Open file given complete file path in SD card
    if (sdcard_open(&fHandle, pcFileName, 1, 0)) {
        DEBUG_PRINTF("avs_record_request(): sdcard_open failed!\r\n");
        audio_mic_end();
        return 0;
    }

#if USE_RECORDPLAY_MUTEX
    xSemaphoreTake(m_xMutexRecordPlay, pdMS_TO_TICKS(portMAX_DELAY));
#endif

    // Record microphone input to SD card while callback function returns true
    // Microphone input is 16-bit stereo
    do {
        // Process transfer if the mic is full
        if (audio_mic_ready()) {

            ulRecordSize = AVS_CONFIG_AUDIO_BUFFER_SIZE;

            // copy data from microphone
            audio_record((uint8_t*)pcMicrophone, ulRecordSize);

            // convert stereo to mono in-place
            audio_stereo_to_mono(pcMicrophone, pcMicrophone, ulRecordSize);
            ulRecordSize = ulRecordSize >> 1;

            // write mic data to SD card
            uint32_t ulWriteSize = 0;
            sdcard_write(&fHandle, pcMicrophone, ulRecordSize, (UINT*)&ulWriteSize);
            if (ulRecordSize != ulWriteSize) {
                DEBUG_PRINTF("avs_record_request(): sdcard_write failed! %d %d\r\n\r\n",
                    (int)ulRecordSize, (int)ulWriteSize);
                audio_mic_clear();
                break;
            }

            // Clear interrupt flag
            audio_mic_clear();

            ulBytesWritten += ulWriteSize;
            //DEBUG_PRINTF("avs_record_request ulBytesWritten %d\r\n", (int)ulWriteSize);
        }
    } while ((*fxnCallbackRecord)() && ulBytesWritten < AVS_CONFIG_MAX_RECORD_SIZE);

#if USE_RECORDPLAY_MUTEX
    xSemaphoreGive(m_xMutexRecordPlay);
#endif

    // Close the file
    sdcard_close(&fHandle);
    DEBUG_PRINTF(">> %s %d bytes (16-bit, %s, mono)\r\n",
        pcFileName, (int)ulBytesWritten, getConfigSamplingRateStr());

    audio_mic_end();

    return 1;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Sends the voice request to the RPI Alexa Gateway provided the filename of the voice recording in the SD card.
// - Read 4KB from SD card
// - Convert 4KB 16-bit to 2KB 8-bit
// - Send 2KB to RPI
// Audio sent: 8-bit u-law, 16KHZ, mono (1-channel)
/////////////////////////////////////////////////////////////////////////////////////////////
int avs_send_request(const char* pcFileName)
{
    FIL fHandle;
    int iRet = 0;
    int iErr = 0;
    char* pcSDCard = g_pcSDCardBuffer;
    uint32_t ulBytesToProcess = AVS_CONFIG_SDCARD_BUFFER_SIZE>>1;
    uint32_t ulBytesToTransfer = 0;
    uint32_t ulBytesSent = 0;


#if USE_DO_DIR
    DEBUG_PRINTF("\r\nChecking SD card directory...\r\n");
    sdcard_dir("");
#endif // USE_DO_DIR

    // Open file given complete file path in SD card
    if (sdcard_open(&fHandle, pcFileName, 0, 1)) {
        DEBUG_PRINTF("avs_send_request(): sdcard_open failed!\r\n");
        return 0;
    }

    // Get file size
    ulBytesToTransfer = sdcard_size(&fHandle);
    if (!ulBytesToTransfer) {
        DEBUG_PRINTF("avs_send_request(): sdcard_size failed!\r\n");
        sdcard_close(&fHandle);
        return 0;
    }
    sdcard_lseek(&fHandle, 0);
    DEBUG_PRINTF(">> %s %d bytes (16-bit)\r\n", pcFileName, (int)ulBytesToTransfer);
    ulBytesToTransfer = (ulBytesToTransfer >> 1);

#if USE_SENDRECV_MUTEX
    xSemaphoreTake(m_xMutexSendRecv, pdMS_TO_TICKS(portMAX_DELAY));
#endif

    // Set a timeout for the operation
    comm_setsockopt(AVS_CONFIG_TX_TIMEOUT, 1);

    // Negotiate the bytes to transfer
    iRet = comm_send((char*)&ulBytesToTransfer, sizeof(ulBytesToTransfer));
    if (iRet != sizeof(ulBytesToTransfer)) {
        DEBUG_PRINTF("avs_send_request(): send failed! %d %d\r\n\r\n", iRet, sizeof(ulBytesToTransfer));
        iRet = 0;
        goto err;
    }


    // Set a timeout for the operation
    comm_setsockopt(AVS_CONFIG_TX_TIMEOUT, 1);

    // Send the flag for configurations
    // Currently the flag is only for specifying the sampling rate of the response
    uint32_t ulFlag = AVS_CONFIG_SAMPLING_RATE;
    iRet = comm_send((char*)&ulFlag, sizeof(ulFlag));
    if (iRet != sizeof(ulFlag)) {
        DEBUG_PRINTF("avs_send_request(): send failed! %d %d\r\n\r\n", iRet, sizeof(ulFlag));
        iRet = 0;
        goto err;
    }

    //DEBUG_PRINTF(">> Sent %d bytes to: ('%s', %d)\r\n", size_tx, addr, port);


    // Set a timeout for the operation
    comm_setsockopt(AVS_CONFIG_TX_TIMEOUT, 1);

    while (ulBytesSent != ulBytesToTransfer) {
        // Compute the transfer size
        if (ulBytesToTransfer-ulBytesSent < ulBytesToProcess) {
            ulBytesToProcess = ulBytesToTransfer-ulBytesSent;
        }

        // Read from SD card
        uint32_t ulReadSize = 0;
        iRet = sdcard_read(&fHandle, pcSDCard, ulBytesToProcess<<1, (UINT*)&ulReadSize);
        if (iRet != 0) {
            DEBUG_PRINTF(">> avs_send_request(): iRet = %d\r\n", iRet);
            iRet = 0;
            goto err;
        }

        if (ulReadSize) {
            // Convert in-place from 16-bit to 8-bit
            audio_pcm16_to_ulaw(ulReadSize, pcSDCard, pcSDCard);

            // Send the converted bytes
            iRet = comm_send(pcSDCard, ulReadSize>>1);
            if (iRet <= 0) {
                DEBUG_PRINTF("avs_send_request(): send failed! %d %d\r\n\r\n", (int)ulReadSize>>1, iRet);
                iRet = 0;
                goto err;
            }
            //DEBUG_PRINTF(">> Sent %d bytes\r\n", size_tx);

            // Compute the total bytes sent
            ulBytesSent += iRet;
        }
        else {
            DEBUG_PRINTF(">> avs_send_request(): sdcard_read is 0\r\n");
        }
    }
    iRet = ulBytesSent;


err:
    // Close the file
    sdcard_close(&fHandle);

#if USE_SENDRECV_MUTEX
    xSemaphoreGive(m_xMutexSendRecv);
#endif

    DEBUG_PRINTF(">> Total bytes sent %d (8-bit compressed)\r\n", (int)ulBytesSent);
    return iRet;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Receives Alexa response and save the uncompressed file to SD card
// - Recv 2KB from RPI
// - Convert 8-bit to 16-bit
// - Save 4KB to SD card
// Audio received: 8-bit u-law, 16KHZ, mono (1-channel)
// Audio saved:   16-bit PCM, 16KHZ, mono (1-channel)
/////////////////////////////////////////////////////////////////////////////////////////////
int avs_recv_response(const char* pcFileName)
{
    FIL fHandle;
    int iRet = 0;
    char* pcSDCard = g_pcSDCardBuffer;
    char* pcRecv = g_pcAudioBuffer;
    uint32_t ulBytesToProcess = AVS_CONFIG_SDCARD_BUFFER_SIZE >> 1;
    uint32_t ulBytesToReceive = 0;
    uint32_t ulBytesReceived = 0;
    uint32_t ulWriteSize = 0;


    // Open file given complete file path in SD card
    if (sdcard_open(&fHandle, pcFileName, 1, 0)) {
        DEBUG_PRINTF("avs_recv_response(): sd_open failed!\r\n");
        return 0;
    }


    // Set a timeout for the operation
    comm_setsockopt(AVS_CONFIG_RX_TIMEOUT, 0);

    // Negotiate the bytes to transfer
    iRet = comm_recv((char*)&ulBytesToReceive, sizeof(ulBytesToReceive));
    if (iRet < sizeof(ulBytesToReceive)) {
        DEBUG_PRINTF("avs_recv_response(): recv failed! %d %d errno %d\r\n\r\n", iRet, sizeof(ulBytesToReceive), comm_errno());
        sdcard_close(&fHandle);
        return 0;
    }
    else if (ulBytesToReceive == 0) {
        // timedout
        return -1;
    }
    DEBUG_PRINTF(">> Total bytes to recv %d (8-bit compressed)\r\n", (int)ulBytesToReceive);


    // Set a timeout for the operation
    comm_setsockopt(AVS_CONFIG_RX_TIMEOUT, 0);

    // Receive the total bytes in segments of buffer size
    do {
        // Compute the transfer size
        if (ulBytesToReceive-ulBytesReceived < ulBytesToProcess) {
            ulBytesToProcess = ulBytesToReceive-ulBytesReceived;
        }

#if USE_SENDRECV_MUTEX
        xSemaphoreTake(m_xMutexSendRecv, pdMS_TO_TICKS(portMAX_DELAY));
#endif

        // Receive the bytes of transfer size
        iRet = comm_recv(pcRecv, ulBytesToProcess);
        if (iRet <= 0) {
            DEBUG_PRINTF("avs_recv_response(): recv failed! %d %d errno %d\r\n\r\n", (int)ulBytesToProcess, iRet, comm_errno());
            sdcard_close(&fHandle);
#if USE_SENDRECV_MUTEX
            xSemaphoreGive(m_xMutexSendRecv);
#endif
            return 0;
        }
        //DEBUG_PRINTF(">> Recv  %d bytes\r\n", iRet);

#if USE_SENDRECV_MUTEX
        xSemaphoreGive(m_xMutexSendRecv);
        vTaskDelay( pdMS_TO_TICKS(1) );
#endif

        // Compute the total bytes received
        ulBytesReceived += iRet;

        // Convert 8-bit data to 16-bit data before saving
        audio_ulaw_to_pcm16(iRet, pcRecv, pcSDCard);

        // Save to 16-bit decoded data to SD card
        ulWriteSize = 0;
        iRet = iRet<<1;
        sdcard_write(&fHandle, pcSDCard, iRet, (UINT*)&ulWriteSize);
        if (iRet != ulWriteSize) {
            DEBUG_PRINTF("avs_recv_response(): sdcard_write failed! %d %d\r\n\r\n", iRet, (int)ulWriteSize);
            sdcard_close(&fHandle);
            return 0;
        }
        //DEBUG_PRINTF(">> Wrote %d bytes to SD card\r\n", ulWriteSize);
    }
    while (ulBytesReceived < ulBytesToReceive);


    // Close the file
    sdcard_close(&fHandle);
    DEBUG_PRINTF(">> %s %d bytes (16-bit)\r\n", pcFileName, (int)ulBytesReceived<<1);

#if USE_DO_DIR
    // f_read fails when sd_dir() is not called for some reason
    DEBUG_PRINTF("\r\nChecking SD card directory...\r\n");
    sdcard_dir("");
#endif // USE_DO_DIR

    return 1;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Play audio file from SD card given the complete file path
// - Read 4KB from SD card
// - For each 1KB, convert from mono (1KB) to stereo (2KB)
// - Play 2KB on speaker
// Audio read:   16-bit PCM, 16KHZ, mono (1-channel)
// Audio played: 16-bit PCM, 16KHZ, stereo (2-channels)
/////////////////////////////////////////////////////////////////////////////////////////////
int avs_play_response(const char* pcFileName)
{
    FIL fHandle;
    uint32_t ulFileSize = 0;
    uint32_t ulFileOffset = 0;
    uint32_t ulReadSize = 0;
    char* pcSpeaker = g_pcAudioBuffer;
    char* pcSDCard = g_pcSDCardBuffer;
    uint32_t ulPlayed = 0;
    uint32_t ulTransferSize = AVS_CONFIG_AUDIO_BUFFER_SIZE<<1;


    audio_speaker_begin();

    // Open file given complete file path in SD card
    if (sdcard_open(&fHandle, pcFileName, 0, 1)) {
        DEBUG_PRINTF("avs_play_response(): sdcard_open failed!\r\n");
        audio_speaker_end();
        return 0;
    }

    // Get file size
    ulFileSize = sdcard_size(&fHandle);
    if (!ulFileSize) {
        DEBUG_PRINTF("avs_play_response(): sdcard_size failed!\r\n");
        sdcard_close(&fHandle);
        audio_speaker_end();
        return 0;
    }
    DEBUG_PRINTF(">> %s %d bytes (16-bit, %s, mono)\r\n",
        pcFileName, (int)ulFileSize, getConfigSamplingRateStr());


    // This code maximizes FIFO sizes of SD CARD (4KB) and SPEAKER (2KB)
    // As a result, transfers are efficient and audio quality is very good
    do {
        // Read 4KB data from SD card to buffer
        ulReadSize = 0;
        sdcard_read(&fHandle, pcSDCard, ulTransferSize, (UINT*)&ulReadSize);
        ulFileOffset += ulReadSize;

        // Write 4KB in 1KB (MONO) chunks (==2KB for STEREO)
        ulPlayed = 0;
        uint32_t ulPlaySize = 1024;
        do {
            // Process transfer if the speaker FIFO is empty
            if (audio_speaker_ready()) {
                if (ulReadSize - ulPlayed < ulPlaySize) {
                    ulPlaySize = ulReadSize - ulPlayed;
                }

                // Input is mono 1 channel; speaker requires stereo 2 channels
                audio_mono_to_stereo(pcSpeaker, pcSDCard + ulPlayed, ulPlaySize);

                // Play buffer to speaker
                audio_play((uint8_t *)pcSpeaker, ulPlaySize<<1);

                // Clear interrupt flag
                audio_speaker_clear();

                // Increment offset
                ulPlayed += ulPlaySize;
            }
        }
        while (ulPlayed < ulReadSize);

        // Compute size to transfer
        if (ulFileSize - ulFileOffset < ulTransferSize) {
            ulTransferSize = ulFileSize - ulFileOffset;
        }

    } while (ulFileOffset != ulFileSize);


    // Close the file
    sdcard_close(&fHandle);

    audio_speaker_end();

    DEBUG_PRINTF(">> Total bytes played %d\r\n", (int)ulFileSize);
    return 1;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Recv and play Alexa response without saving to SD card
// - Recv 512 bytes from RPI
// - Convert 8-bit mono (512) to 16-bit stereo (2KB)
// - Play 2KB on speaker
/////////////////////////////////////////////////////////////////////////////////////////////
int avs_recv_and_play_response(void)
{
    int iRet = 0;
    char* pcRecv = g_pcSDCardBuffer;
    char* pcSpeaker = g_pcAudioBuffer;
    uint32_t ulBytesToProcess = AVS_CONFIG_AUDIO_BUFFER_SIZE>>2;
    uint32_t ulBytesToReceive = 0;
    uint32_t ulBytesReceived = 0;


    // Set a timeout for the operation
    comm_setsockopt(AVS_CONFIG_RX_TIMEOUT, 0);

    // Negotiate the bytes to transfer
    iRet = comm_recv((char*)&ulBytesToReceive, sizeof(ulBytesToReceive));
    if (iRet < sizeof(ulBytesToReceive)) {
        tfp_printf("avs_recv_and_play_response(): recv failed! %d %d errno %d\r\n\r\n", iRet, sizeof(ulBytesToReceive), comm_errno());
        return 0;
    }
    else if (ulBytesToReceive == 0) {
        // timedout
        return -1;
    }
    DEBUG_PRINTF(">> Total bytes to recv %d (8-bit compressed)\r\n", (int)ulBytesToReceive);


#if USE_RECORDPLAY_MUTEX
    xSemaphoreTake(m_xMutexRecordPlay, pdMS_TO_TICKS(portMAX_DELAY));
#endif // USE_RECORDPLAY_MUTEX

    audio_speaker_begin();

    // Receive the total bytes in segments of buffer size
    do {
#if USE_SENDRECV_MUTEX
        xSemaphoreTake(m_xMutexSendRecv, pdMS_TO_TICKS(portMAX_DELAY));
#endif // USE_SENDRECV_MUTEX

        // Set a timeout for the operation
        comm_setsockopt(AVS_CONFIG_RX_TIMEOUT, 0);

        // Receive the bytes of transfer size
        iRet = comm_recv(pcRecv, ulBytesToProcess);
        if (iRet <= 0) {
            DEBUG_PRINTF("avs_recv_and_play_response(): recv2 failed! %d %d errno %d\r\n\r\n", (int)ulBytesToProcess, iRet, comm_errno());
#if USE_SENDRECV_MUTEX
            xSemaphoreGive(m_xMutexSendRecv);
#endif // USE_SENDRECV_MUTEX
            ulBytesReceived = 0;
            break;
        }
        DEBUG_PRINTF(">> Recv  %d bytes\r\n", iRet);

#if USE_SENDRECV_MUTEX
        xSemaphoreGive(m_xMutexSendRecv);
        vTaskDelay( pdMS_TO_TICKS(1) );
#endif // USE_SENDRECV_MUTEX


        // Compute the total bytes received
        ulBytesReceived += iRet;

        // Convert 8-bit mono data to 16-bit stereo data before saving
        audio_ulaw_to_pcm16_stereo(iRet, pcRecv, pcSpeaker);

        // Play on speaker
        do {
            if (audio_speaker_ready()) {
                audio_play((uint8_t *)pcSpeaker, iRet<<2);
                audio_speaker_clear();
                break;
            }
        } while (1);

        // Compute the transfer size
        if (ulBytesToReceive-ulBytesReceived < ulBytesToProcess) {
            ulBytesToProcess = ulBytesToReceive-ulBytesReceived;
        }
    }
    while (ulBytesReceived < ulBytesToReceive);


    audio_speaker_end();

#if USE_RECORDPLAY_MUTEX
    xSemaphoreGive(m_xMutexRecordPlay);
#endif // USE_RECORDPLAY_MUTEX

    return ulBytesReceived;
}


#if USE_MULTITHREADED_RECVPLAY
/////////////////////////////////////////////////////////////////////////////////////////////
// Recv and play Alexa response in separate threads
// - Recv 2KB from RPI
// - Convert 8-bit to 16-bit (1KB)
// - Save 4KB to SD card
/////////////////////////////////////////////////////////////////////////////////////////////
int avs_recv_and_play_response_threaded(const char* pcFileName)
{
    int iRet = 0;
    int iSignalled = 0;
    char acSDCard[AVS_CONFIG_SDCARD_BUFFER_SIZE];
    char acRecv[AVS_CONFIG_SDCARD_BUFFER_SIZE>>1];
    uint32_t ulBytesToProcess = sizeof(acRecv);
    uint32_t ulBytesReceived = 0;


    // Initialize structure
    g_hContext.m_ulWriteSize = 0;
    g_hContext.m_ulReadSize = 0;
    g_hContext.m_ulRecvSize = 0;

    // Open file given complete file path in SD card
    if (sdcard_open(&g_hContext.m_fHandle, pcFileName, 1, 1)) {
        DEBUG_PRINTF("avs_recv_and_play_response_threaded(): sd_open failed!\r\n");
        return 0;
    }


    // Set a timeout for the operation
    comm_setsockopt(AVS_CONFIG_RX_TIMEOUT, 0);

    // Negotiate the bytes to transfer
    iRet = comm_recv((char*)&g_hContext.m_ulRecvSize, sizeof(g_hContext.m_ulRecvSize));
    if (iRet < sizeof(g_hContext.m_ulRecvSize)) {
        DEBUG_PRINTF("avs_recv_and_play_response_threaded(): recv failed! %d %d\r\n\r\n", iRet, sizeof(g_hContext.m_ulRecvSize));
        sdcard_close(&g_hContext.m_fHandle);
        return 0;
    }
    DEBUG_PRINTF(">> Total bytes to recv %d (8-bit compressed)\r\n", (int)g_hContext.m_ulRecvSize);


    // Set a timeout for the operation
    comm_setsockopt(AVS_CONFIG_RX_TIMEOUT, 0);

    // Receive the total bytes in segments of buffer size
    do {
        // Compute the transfer size
        ulBytesToProcess = sizeof(acRecv);
        if (g_hContext.m_ulRecvSize - ulBytesReceived < ulBytesToProcess) {
            ulBytesToProcess = g_hContext.m_ulRecvSize - ulBytesReceived;
        }

        // Save to 16-bit decoded data to SD card
        // Take the semaphore
        if (xSemaphoreTake(g_hContext.m_xMutexFile, pdMS_TO_TICKS(1000)) == pdTRUE) {

            // Receive the bytes of transfer size
            iRet = comm_recv(acRecv, ulBytesToProcess);
            if (iRet <= 0) {
                DEBUG_PRINTF("avs_recv_and_play_response_threaded(): recv failed! %d %d\r\n\r\n", (int)ulBytesToProcess, iRet);
                if (ulBytesReceived) {
                    xSemaphoreTake(g_hContext.m_xMutexRun, pdMS_TO_TICKS(portMAX_DELAY));
                }
                sdcard_close(&g_hContext.m_fHandle);
                return 0;
            }
            //tfp_printf(">> Recv  %d bytes %d\r\n", iRet, g_hContext.m_ulWriteSize);

            // Convert 8-bit data to 16-bit data before saving
            audio_ulaw_to_pcm16(iRet, acRecv, acSDCard);

            /* Get write position */
            sdcard_lseek(&g_hContext.m_fHandle, g_hContext.m_ulWriteSize);
            
            /* Write data to specified position */
            uint32_t ulWriteSize = 0;
            int lSizeToWrite = iRet<<1;
            sdcard_write(&g_hContext.m_fHandle, acSDCard, lSizeToWrite, (UINT*)&ulWriteSize);
            if (lSizeToWrite != ulWriteSize) {
                DEBUG_PRINTF("avs_recv_and_play_response_threaded(): sdcard_write failed! %d %d\r\n\r\n", lSizeToWrite, (int)ulWriteSize);
                xSemaphoreGive(g_hContext.m_xMutexFile);
                if (ulBytesReceived) {
                    xSemaphoreTake(g_hContext.m_xMutexRun, pdMS_TO_TICKS(portMAX_DELAY));
                }
                sdcard_close(&g_hContext.m_fHandle);
                return 0;
            }

            g_hContext.m_ulWriteSize += ulWriteSize;

            /* Release semaphone */
            xSemaphoreGive(g_hContext.m_xMutexFile);
            vTaskDelay(pdMS_TO_TICKS(1)); // Allow context switch
        }
        else {
            DEBUG_PRINTF("avs_recv_and_play_response_threaded(): xSemaphoreTake failed!\r\n\r\n");
        }

        // Compute the total bytes received
        ulBytesReceived += iRet;

        // Signal other thread to start
        if (g_hContext.m_ulWriteSize>=4096 && !iSignalled) {
            xSemaphoreGive(g_hContext.m_xMutexRun);
            iSignalled = 1;
        }
    }
    while (ulBytesReceived < g_hContext.m_ulRecvSize);

    DEBUG_PRINTF(">> %s %d bytes (16-bit)\r\n", pcFileName, (int)ulBytesReceived<<1);

 
    xSemaphoreTake(g_hContext.m_xMutexRun, pdMS_TO_TICKS(portMAX_DELAY));
    // Wait for semaphore before closing it
    while (g_hContext.m_ulReadSize != g_hContext.m_ulWriteSize) {
        vTaskDelay( pdMS_TO_TICKS(1) );
    }

    // Close the file
    sdcard_close(&g_hContext.m_fHandle);

    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Helper for avs_recv_and_play_response_threaded
// - Read 4KB from SD card
// - For each 1KB, convert from mono (1KB) to stereo (2KB)
// - Play 2KB on speaker
/////////////////////////////////////////////////////////////////////////////////////////////
static void vPlayerTask(void *pvParameters)
{
    ThreadPlayerContext* pContext = (ThreadPlayerContext*)pvParameters;
    char* pcSDCard = g_pcSDCardBuffer;
    char* pcSpeaker = g_pcAudioBuffer;
    uint32_t ulTransferSize = 0;
    uint32_t ulReadSize = 0;


    while (1) {

        // wait for signal
        xSemaphoreTake(pContext->m_xMutexRun, pdMS_TO_TICKS(portMAX_DELAY));

        audio_speaker_begin();

        while (1) {

            // Take the semaphore
            if (xSemaphoreTake(pContext->m_xMutexFile, pdMS_TO_TICKS(1000)) == pdTRUE) {

                // Check if new written bytes are available for reading
                if (pContext->m_ulWriteSize == 0 ||
                    (pContext->m_ulWriteSize - pContext->m_ulReadSize < 4096 &&
                     pContext->m_ulWriteSize < (pContext->m_ulRecvSize << 1) ) ) {
                    xSemaphoreGive(pContext->m_xMutexFile);
                    vTaskDelay(pdMS_TO_TICKS(1)); // Allow context switch
                    continue;
                }

                // Compute size to transfer
                ulTransferSize = AVS_CONFIG_SDCARD_BUFFER_SIZE;
                if (pContext->m_ulWriteSize - pContext->m_ulReadSize < ulTransferSize) {
                    ulTransferSize = pContext->m_ulWriteSize - pContext->m_ulReadSize;
                }

                // Get read position
                sdcard_lseek(&pContext->m_fHandle, pContext->m_ulReadSize);

                // Read data from SD card to buffer
                ulReadSize = 0;
                sdcard_read(&pContext->m_fHandle, pcSDCard, ulTransferSize, (UINT*)&ulReadSize);
                //tfp_printf(">> Read  %d bytes %d\r\n", (int)ulReadSize, (int)g_hContext.m_ulReadSize);

                // Increment bytes read
                pContext->m_ulReadSize += ulReadSize;

                // Write 4KB in 1KB (MONO) chunks (==2KB for STEREO)
                uint32_t ulPlayed = 0;
                uint32_t ulPlaySize = 1024;
                do {
                    // Process transfer if the speaker FIFO is empty
                    if (audio_speaker_ready()) {
                        if (ulReadSize - ulPlayed < ulPlaySize) {
                            ulPlaySize = ulReadSize - ulPlayed;
                        }

                        // Input is mono 1 channel; speaker requires stereo 2 channels
                        audio_mono_to_stereo(pcSpeaker, pcSDCard + ulPlayed, ulPlaySize);

                        // Play buffer to speaker
                        audio_play((uint8_t *)pcSpeaker, ulPlaySize<<1);
                        // Increment offset
                        ulPlayed += ulPlaySize;

                        audio_speaker_clear();
                    }
                }
                while (ulPlayed < ulReadSize);

                // Release semaphone
                xSemaphoreGive(pContext->m_xMutexFile);
                vTaskDelay(pdMS_TO_TICKS(1)); // Allow context switch


                // Check if all bytes are read
                if ((pContext->m_ulRecvSize << 1) <= pContext->m_ulReadSize) {
                    break;
                }
            }
            else {
                DEBUG_PRINTF("avs_recv_and_play_response_threaded(): xSemaphoreTake failed!\r\n\r\n");
            }
        }
        
        // Release semaphone
        xSemaphoreGive(pContext->m_xMutexRun);

        audio_speaker_end();

        vTaskDelay( pdMS_TO_TICKS(1000) ); // Allow context switch
    }
}
#endif

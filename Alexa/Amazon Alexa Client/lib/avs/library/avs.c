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
#include "tinyprintf.h"    // tinyprintf 3rd-party library
#include "FreeRTOS.h"      // FreeRTOS 3rd-party library
#include "task.h"          // FreeRTOS 3rd-party library
#include "lwip/sockets.h"  // lwIP 3rd-party library

#include "avs/avs.h"       // AVS library
#include "avs_config.h"    // AVS configuration
#include "utils/audio.h"   // Audio utility
#include "utils/sdcard.h"  // SD card utility
#include "utils/ulaw.h"    // Audio compression/expansion utility



//#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(...) do {tfp_printf(__VA_ARGS__);} while (0)
#else
#define DEBUG_PRINTF(...)
#endif



#define USE_DO_DIR 0
#define USE_STEREO_TO_MONO_AVERAGE 0 // Using discard instead of average is better audio quality.



static int   g_lSocket        = -1;
static char* g_pcTxRxBuffer   = NULL;
static char* g_pcAudioBuffer  = NULL;



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
#endif

int avs_init()
{
    // Initialize audio
    audio_setup(NULL, AVS_CONFIG_SAMPLING_RATE);
    DEBUG_PRINTF("Audio initialize.\r\n");

    // Initialize SD card
    sdcard_setup();
    DEBUG_PRINTF("SDCard initialize.\r\n");

    // Pre-allocate the buffers now for faster communication
    g_pcTxRxBuffer = pvPortMalloc(AVS_CONFIG_RXTX_BUFFER_SIZE);
    g_pcAudioBuffer = pvPortMalloc(AVS_CONFIG_AUDIO_BUFFER_SIZE);
    if (!g_pcTxRxBuffer || !g_pcAudioBuffer) {
        DEBUG_PRINTF("avs_init(): pvPortMalloc failed %p %p\n",
            g_pcTxRxBuffer, g_pcAudioBuffer);
        return 0;
    }
    DEBUG_PRINTF("Memory initialize.\r\n");

    return 1;
}

void avs_free()
{
    if (g_pcTxRxBuffer) {
        vPortFree(g_pcTxRxBuffer);
        g_pcTxRxBuffer = NULL;
    }

    if (g_pcAudioBuffer) {
        vPortFree(g_pcAudioBuffer);
        g_pcAudioBuffer = NULL;
    }
}

int avs_get_server_port()
{
    return AVS_CONFIG_SERVER_PORT;
}

const ip_addr_t* avs_get_server_addr()
{
    static struct sockaddr_in tServer;
    tServer.sin_addr.s_addr = AVS_CONFIG_SERVER_ADDR;
    return (ip_addr_t*)&tServer.sin_addr;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Establishes connection to the RPI Alexa Gateway using configurations in avs_config.h configuration file.
/////////////////////////////////////////////////////////////////////////////////////////////
int avs_connect()
{
    int iRet = 0;
    struct sockaddr_in tServer = {0};


    // Set server info
    tServer.sin_family = AF_INET;
    tServer.sin_port = htons(AVS_CONFIG_SERVER_PORT);
    tServer.sin_addr.s_addr = AVS_CONFIG_SERVER_ADDR;

    // Create a TCP socket
    if ((g_lSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        DEBUG_PRINTF("avs_connect(): socket failed!\r\n");
        chip_reboot();
        return 0;
    }

    // Connect to server
    if ((iRet = connect(g_lSocket, (struct sockaddr *) &tServer, sizeof(tServer))) < 0) {
        avs_disconnect();
        return 0;
    }

    return 1;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Closes connection with RPI Alexa Gateway.
/////////////////////////////////////////////////////////////////////////////////////////////
void avs_disconnect()
{
    if (g_lSocket >= 0) {
        close(g_lSocket);
        shutdown(g_lSocket, SHUT_RDWR);
        g_lSocket = -1;
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Sends the voice request to the RPI Alexa Gateway provided the filename of the voice recording in the SD card.
// Audio sent: 8-bit u-law, 16KHZ, mono (1-channel)
/////////////////////////////////////////////////////////////////////////////////////////////
int avs_send_request(const char* pcFileName)
{
    FIL fHandle;
    int iRet = 0;
    char* pcData = g_pcTxRxBuffer;
    uint32_t ulBytesToTransfer = 0;
    uint32_t ulBytesToProcess = AVS_CONFIG_TX_SIZE;
    uint32_t ulBytesSent = 0;
    struct timeval tTimeout = {AVS_CONFIG_TX_TIMEOUT, 0}; // x-second timeout


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


    // Set a X-second timeout for the operation
    setsockopt(g_lSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tTimeout, sizeof(tTimeout));

    // Negotiate the bytes to transfer
    iRet = send(g_lSocket, (char*)&ulBytesToTransfer, sizeof(ulBytesToTransfer), 0);
    if (iRet != sizeof(ulBytesToTransfer)) {
        DEBUG_PRINTF("avs_send_request(): send failed! %d %d\r\n\r\n", iRet, sizeof(ulBytesToTransfer));
        sdcard_close(&fHandle);
        return 0;
    }


    // Set a X-second timeout for the operation
    setsockopt(g_lSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tTimeout, sizeof(tTimeout));

    // Send the flag for configurations
    // Currently the flag is only for specifying the sampling rate of the response
    uint32_t ulFlag = AVS_CONFIG_SAMPLING_RATE;
    iRet = send(g_lSocket, (char*)&ulFlag, sizeof(ulFlag), 0);
    if (iRet != sizeof(ulFlag)) {
        DEBUG_PRINTF("avs_send_request(): send failed! %d %d\r\n\r\n", iRet, sizeof(ulFlag));
        sdcard_close(&fHandle);
        return 0;
    }

    //DEBUG_PRINTF(">> Sent %d bytes to: ('%s', %d)\r\n", size_tx, addr, port);


    // Set a X-second timeout for the operation
    setsockopt(g_lSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tTimeout, sizeof(tTimeout));

    while (ulBytesSent != ulBytesToTransfer) {
        // Compute the transfer size
        if (ulBytesToTransfer-ulBytesSent < ulBytesToProcess) {
            ulBytesToProcess = ulBytesToTransfer-ulBytesSent;
        }

        // Read from SD card
        uint32_t ulReadSize = 0;
        iRet = sdcard_read(&fHandle, pcData, ulBytesToProcess<<1, (UINT*)&ulReadSize);
        ulBytesToProcess = ulReadSize >> 1;
        if (iRet != 0) {
            DEBUG_PRINTF(">> avs_send_request(): iRet = %d\r\n", iRet);
            sdcard_close(&fHandle);
            return 0;
        }

        if (ulBytesToProcess) {
            // Convert in-place from 16-bit to 8-bit
            pcm16_to_ulaw(ulBytesToProcess<<1, pcData, pcData);

            // Send the converted bytes
            iRet = send(g_lSocket, pcData, ulBytesToProcess, 0);
            if (iRet <= 0) {
                DEBUG_PRINTF("avs_send_request(): send failed! %d %d\r\n\r\n", (int)ulBytesToProcess, iRet);
                sdcard_close(&fHandle);
                return 0;
            }
            //DEBUG_PRINTF(">> Sent %d bytes\r\n", size_tx);

            // Compute the total bytes sent
            ulBytesSent += iRet;
        }
        else {
            DEBUG_PRINTF(">> avs_send_request(): sdcard_read is 0\r\n");
        }
    }


    // Close the file
    sdcard_close(&fHandle);

    DEBUG_PRINTF(">> Total bytes sent %d (8-bit compressed)\r\n", (int)ulBytesSent);
    return 1;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Receives Alexa response and save the uncompressed file to SD card
// - Uses a pre-allocated buffer for faster transfer
// - Receives 1460 bytes from RPI then uncompresses it from 8-bit to 16-bit using ulaw algorithm
//   then saves the converted audio data stream of size 1460*2 bytes to SD card
// Audio received: 8-bit u-law, 16KHZ, mono (1-channel)
// Audio saved:   16-bit PCM, 16KHZ, mono (1-channel)
/////////////////////////////////////////////////////////////////////////////////////////////
int avs_recv_response(const char* pcFileName)
{
    FIL fHandle;
    int iRet = 0;
    char* pcData = g_pcTxRxBuffer;
    char acTemp[AVS_CONFIG_RX_SIZE];
    uint32_t ulBytesToReceive = 0;
    uint32_t ulBytesReceived = 0;
    uint32_t ulBytesToProcess = sizeof(acTemp);
    struct timeval tTimeout = {AVS_CONFIG_RX_TIMEOUT, 0}; // x-second timeout


    // Open file given complete file path in SD card
    if (sdcard_open(&fHandle, pcFileName, 1, 0)) {
        DEBUG_PRINTF("avs_recv_response(): sd_open failed!\r\n");
        return 0;
    }


    // Set a 15-second timeout for the operation
    setsockopt(g_lSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tTimeout, sizeof(tTimeout));

    // Negotiate the bytes to transfer
    iRet = recv(g_lSocket, (char*)&ulBytesToReceive, sizeof(ulBytesToReceive), 0);
    if (iRet < sizeof(ulBytesToReceive)) {
        DEBUG_PRINTF("avs_recv_response(): recv failed! %d %d\r\n\r\n", iRet, sizeof(ulBytesToReceive));
        sdcard_close(&fHandle);
        return 0;
    }
    DEBUG_PRINTF(">> Total bytes to recv %d (8-bit compressed)\r\n", (int)ulBytesToReceive);


    // Set a 15-second timeout for the operation
    setsockopt(g_lSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tTimeout, sizeof(tTimeout));

    // Receive the total bytes in segments of buffer size
    while (ulBytesReceived < ulBytesToReceive) {
        // Compute the transfer size
        if (ulBytesToReceive-ulBytesReceived < ulBytesToProcess) {
            ulBytesToProcess = ulBytesToReceive-ulBytesReceived;
        }

        // Receive the bytes of transfer size
        iRet = recv(g_lSocket, acTemp, ulBytesToProcess, 0);
        if (iRet <= 0) {
            DEBUG_PRINTF("avs_recv_response(): recv failed! %d %d\r\n\r\n", (int)ulBytesToProcess, iRet);
            sdcard_close(&fHandle);
            return 0;
        }
        //DEBUG_PRINTF(">> Recv  %d bytes\r\n", iRet);

        // Compute the total bytes received
        ulBytesReceived += iRet;

        // Convert 8-bit data to 16-bit data before saving
        ulaw_to_pcm16(iRet, acTemp, pcData);

        // Save to 16-bit decoded data to SD card
        uint32_t ulWriteSize = 0;
        iRet = iRet<<1;
        sdcard_write(&fHandle, pcData, iRet, (UINT*)&ulWriteSize);
        if (iRet != ulWriteSize) {
            DEBUG_PRINTF("avs_recv_response(): sdcard_write failed! %d %d\r\n\r\n", iRet, (int)ulWriteSize);
            sdcard_close(&fHandle);
            return 0;
        }
        //DEBUG_PRINTF(">> Wrote %d bytes to SD card\r\n", ulWriteSize);
    }


    // Close the file
    sdcard_close(&fHandle);
    DEBUG_PRINTF(">> %s %d bytes (16-bit)\r\n", pcFileName, (int)ulBytesReceived<<1);

#if USE_DO_DIR
    // f_read fails when sd_dir() is not called for some reason
    DEBUG_PRINTF("\r\nChecking SD card directory...\r\n");
    sdcard_dir("");
#endif // USE_DO_DIR

    return ulBytesReceived<<1;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Play audio file from SD card given the complete file path
// - Uses a pre-allocated buffer for faster transfer
// - Reads 1KB segment from file then converts it to stereo (2-channel) audio
//   by duplicating each short then transfers the 2KB segment to I2S Master
// Audio read:   16-bit PCM, 16KHZ, mono (1-channel)
// Audio played: 16-bit PCM, 16KHZ, stereo (2-channels)
/////////////////////////////////////////////////////////////////////////////////////////////
int avs_play_response(const char* pcFileName)
{
    FIL fHandle;
    uint32_t ulFileSize = 0;
    uint32_t ulFileOffset = 0;
    uint32_t ulReadSize = 0;
    uint32_t ulTransferSize = 0;
    char* pcData = g_pcAudioBuffer;


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


#if 1
    // This code maximizes FIFO sizes of SD CARD (4KB) and SPEAKER (2KB)
    // As a result, audio quality is very good
    char* pcSDCard = g_pcTxRxBuffer;
    uint32_t ulPlayed = 0;

    do {
        // Compute size to transfer
        ulTransferSize = AVS_CONFIG_AUDIO_BUFFER_SIZE<<1;
        if (ulFileSize - ulFileOffset < ulTransferSize) {
            ulTransferSize = ulFileSize - ulFileOffset;
        }

        // Read 4KB data from SD card to buffer
        // SD Card FIFO size is 4KB so this is the efficient read size
        ulReadSize = 0;
        sdcard_read(&fHandle, pcSDCard, ulTransferSize, (UINT*)&ulReadSize);
        ulFileOffset += ulReadSize;
        //DEBUG_PRINTF(">> avsPlayAlexaResponse fileOffset %d totalReadSize %d\r\n", fileOffset, totalReadSize);

        // Write the 4KB data to speaker
        // Speaker FIFO size is 2KB only
        // Data is MONO so must be converted to STEREO
        // So write 4KB in 1KB chunks (==2KB for STEREO mode)
        ulPlayed = 0;
        ulTransferSize = 1024;
        while (ulPlayed < ulReadSize) {
            // Process transfer if the speaker FIFO is empty
            if (audio_speaker_ready()) {
                // Duplicate short for stereo 2 channel
                // Input is mono 1 channel audio data stream
                // I2S requires stereo 2 channel audio data stream
                char* pDst = pcData;
                char* pSrc = pcSDCard + ulPlayed;
                for (int i=0; i<ulTransferSize; i+=2, pDst+=4, pSrc+=2) {
                    *((uint16_t*)&pDst[0]) = *((uint16_t*)&pSrc[0]);
                    *((uint16_t*)&pDst[2]) = *((uint16_t*)&pDst[0]);
                }

                // Play buffer to speaker
                audio_play((uint8_t *)pcData, ulTransferSize<<1);

                // Clear interrupt flag
                audio_speaker_clear();

                // Increment offset
                ulPlayed += ulTransferSize;

                if (ulReadSize - ulPlayed < ulTransferSize) {
                    ulTransferSize = ulReadSize - ulPlayed;
                }
            }
        }
    } while (ulFileOffset != ulFileSize);
#else
    char acTemp[AVS_CONFIG_AUDIO_BUFFER_SIZE >> 1];
    do {
        // Process transfer if the speaker FIFO is empty
        if (audio_speaker_ready()) {

            // Compute size to transfer
            ulTransferSize = sizeof(acTemp);
            if (ulFileSize - ulFileOffset < ulTransferSize) {
                ulTransferSize = ulFileSize - ulFileOffset;
            }

            // Read data from SD card to buffer
            ulReadSize = 0;
            sdcard_read(&fHandle, acTemp, ulTransferSize, (UINT*)&ulReadSize);
            ulTransferSize = ulReadSize;
            //DEBUG_PRINTF(">> avsPlayAlexaResponse fileOffset %d totalReadSize %d\r\n", fileOffset, totalReadSize);

            if (ulTransferSize) {
                // Duplicate short for stereo 2 channel
                // Input is mono 1 channel audio data stream
                // I2S requires stereo 2 channel audio data stream
                for (int i=0, j=0; i<ulTransferSize; i+=2, j+=4) {
                    pcData[j]   = acTemp[i];
                    pcData[j+1] = acTemp[i+1];
                    pcData[j+2] = acTemp[i];
                    pcData[j+3] = acTemp[i+1];
                }

                // Play buffer to speaker
                audio_play((uint8_t*)pcData, ulTransferSize<<1);

                // Increment offset
                ulFileOffset += ulTransferSize;
            }
            else {
                DEBUG_PRINTF(">> avs_play_response sdcard_read is 0\r\n");
            }

            // Clear interrupt flag
            audio_speaker_clear();
        }
    } while (ulFileOffset != ulFileSize);
#endif


    // Close the file
    sdcard_close(&fHandle);

    audio_speaker_end();

    DEBUG_PRINTF(">> Total bytes played %d\r\n", (int)ulFileSize);
    return 1;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Recv and play Alexa response without saving to SD card
// - Recv 512 bytes from RPI
// - Convert 8-bit to 16-bit (1KB)
// - Convert mono to stereo (2KB)
/////////////////////////////////////////////////////////////////////////////////////////////
int avs_recv_and_play_response()
{
    int iRet = 0;
    char* pcData = g_pcTxRxBuffer;
    char acRecv[512];
    char acExpanded[1024];
    uint32_t ulBytesToReceive = 0;
    uint32_t ulBytesReceived = 0;
    uint32_t ulBytesToProcess = sizeof(acRecv);
    struct timeval tTimeout = {AVS_CONFIG_RX_TIMEOUT, 0}; // x-second timeout


    audio_speaker_begin();

    // Set a 15-second timeout for the operation
    setsockopt(g_lSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tTimeout, sizeof(tTimeout));

    // Negotiate the bytes to transfer
    iRet = recv(g_lSocket, (char*)&ulBytesToReceive, sizeof(ulBytesToReceive), 0);
    if (iRet < sizeof(ulBytesToReceive)) {
        DEBUG_PRINTF("avs_recv_response(): recv failed! %d %d\r\n\r\n", iRet, sizeof(ulBytesToReceive));
        audio_speaker_end();
        return 0;
    }
    DEBUG_PRINTF(">> Total bytes to recv %d (8-bit compressed)\r\n", (int)ulBytesToReceive);


    // Set a 15-second timeout for the operation
    setsockopt(g_lSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tTimeout, sizeof(tTimeout));

    // Receive the total bytes in segments of buffer size
    while (ulBytesReceived < ulBytesToReceive) {
        // Compute the transfer size
        if (ulBytesToReceive-ulBytesReceived < ulBytesToProcess) {
            ulBytesToProcess = ulBytesToReceive-ulBytesReceived;
        }

        // Receive the bytes of transfer size
        iRet = recv(g_lSocket, acRecv, ulBytesToProcess, 0);
        if (iRet <= 0) {
            DEBUG_PRINTF("avs_recv_response(): recv failed! %d %d\r\n\r\n", (int)ulBytesToProcess, iRet);
            audio_speaker_end();
            return 0;
        }
        DEBUG_PRINTF(">> Recv  %d bytes\r\n", iRet);

        // Compute the total bytes received
        ulBytesReceived += iRet;

        // Convert 8-bit data to 16-bit data before saving
        ulaw_to_pcm16(iRet, acRecv, acExpanded);
        iRet = iRet<<1;

        do {
            if (audio_speaker_ready()) {
                // Duplicate short for stereo 2 channel
                // Input is mono 1 channel audio data stream
                // I2S requires stereo 2 channel audio data stream
                char* pDst = pcData;
                char* pSrc = acExpanded;
                for (int i=0; i<iRet; i+=2, pDst+=4, pSrc+=2) {
                    *((uint16_t*)&pDst[0]) = *((uint16_t*)&pSrc[0]);
                    *((uint16_t*)&pDst[2]) = *((uint16_t*)&pDst[0]);
                }

                // Play buffer to speaker
                audio_play((uint8_t *)pcData, iRet<<1);

                // Clear interrupt flag
                audio_speaker_clear();
                break;
            }
        } while (1);
    }

    audio_speaker_end();
    return 1;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Record audio file from microphone and save to SD card given the complete file path
// Audio recorded: 16-bit PCM, 16KHZ, stereo (2-channels)
// Audio saved:    16-bit PCM, 16KHZ, mono (1-channel)
/////////////////////////////////////////////////////////////////////////////////////////////
int avs_record_request(const char* pcFileName, int (*fxnCallbackRecord)(void))
{
    FIL fHandle;
    uint32_t ulRecordSize = 0;
    uint32_t ulBytesWritten = 0;
    char* pcData = g_pcAudioBuffer;


    audio_mic_begin();

    // Open file given complete file path in SD card
    if (sdcard_open(&fHandle, pcFileName, 1, 0)) {
        DEBUG_PRINTF("avs_record_request(): sdcard_open failed!\r\n");
        audio_mic_end();
        return 0;
    }


    // Record microphone input to SD card while callback function returns true
    // Microphone input is 16-bit stereo
    do {
        // Process transfer if the mic is full
        if (audio_mic_ready()) {

            ulRecordSize = AVS_CONFIG_AUDIO_BUFFER_SIZE;

            // copy data from microphone
            audio_record((uint8_t*)pcData, ulRecordSize);

            // convert stereo to mono in-place
            char* pDst = pcData;
            char* pSrc = pcData;
            for (int i=0; i<ulRecordSize; i+=2, pDst+=2, pSrc+=4) {
                // copy the first 16-bit word, skip the next one
                *((uint16_t*)&pDst[0]) = *((uint16_t*)&pSrc[0]);
            }
            ulRecordSize = ulRecordSize >> 1;

            // write mic data to SD card
            uint32_t ulWriteSize = 0;
            sdcard_write(&fHandle, pcData, ulRecordSize, (UINT*)&ulWriteSize);
            if (ulRecordSize != ulWriteSize) {
                DEBUG_PRINTF("avs_record_request(): sdcard_write failed! %d %d\r\n\r\n",
                    (int)ulRecordSize, (int)ulWriteSize);
                audio_mic_clear();
                break;
            }

            // Clear interrupt flag
            audio_mic_clear();

            ulBytesWritten += ulWriteSize;
            //DEBUG_PRINTF("avsRecordAlexaRequest ulBytesWritten %d\r\n", (int)ulWriteSize);
        }
    } while ((*fxnCallbackRecord)() && ulBytesWritten < AVS_CONFIG_MAX_RECORD_SIZE);


    // Close the file
    sdcard_close(&fHandle);
    DEBUG_PRINTF(">> %s %d bytes (16-bit, %s, mono)\r\n",
        pcFileName, (int)ulBytesWritten, getConfigSamplingRateStr());

    audio_mic_end();

    return 1;
}



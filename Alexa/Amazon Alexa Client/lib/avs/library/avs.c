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
#include "lwip/sockets.h"  // lwIP 3rd-party library

#include "avs/avs.h"       // AVS library
#include "avs_config.h"    // AVS configuration
#include "utils/speaker.h" // Speaker utility
#include "utils/sdcard.h"  // SD card utility
#include "utils/ulaw.h"    // Audio compression/expansion utility



#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(...) do {tfp_printf(__VA_ARGS__);} while (0)
#else
#define DEBUG_PRINTF(...)
#endif



static char* g_pcTxRxBuffer  = NULL;
static char* g_pcAudioBuffer = NULL;



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
    else if (AVS_CONFIG_SAMPLING_RATE == SAMPLING_RATE_8KHZ) {
        return "8KHz";
    }

    return "Unknown";
}

void avsInit()
{
    // Initialize speaker
    speaker_setup(NULL, AVS_CONFIG_SAMPLING_RATE);
    DEBUG_PRINTF("Speaker initialize.\r\n\r\n");

    // Initialize SD card
    sdcard_setup();
    DEBUG_PRINTF("SDCard initialize.\r\n\r\n");

    // Pre-allocate the buffers now for faster communication
    g_pcTxRxBuffer = pvPortMalloc(AVS_CONFIG_RXTX_BUFFER_SIZE);
    g_pcAudioBuffer = pvPortMalloc(AVS_CONFIG_AUDIO_BUFFER_SIZE);
    if (!g_pcTxRxBuffer || !g_pcAudioBuffer) {
        DEBUG_PRINTF("pvPortMalloc failed %p %p\n", g_pcTxRxBuffer, g_pcAudioBuffer);
        return;
    }

    // Start the I2S audio data streaming
    speaker_begin();
}

void avsFree()
{
    // Stop the I2S audio data streaming
	speaker_end();

    if (g_pcTxRxBuffer) {
        vPortFree(g_pcTxRxBuffer);
        g_pcTxRxBuffer = NULL;
    }

    if (g_pcAudioBuffer) {
        vPortFree(g_pcAudioBuffer);
        g_pcAudioBuffer = NULL;
    }
}

int avsGetServerPort()
{
    return AVS_CONFIG_SERVER_PORT;
}

const ip_addr_t* avsGetServerAddress()
{
    static struct sockaddr_in tServer;
    tServer.sin_addr.s_addr = AVS_CONFIG_SERVER_ADDR;
    return (ip_addr_t*)&tServer.sin_addr;
}

int avsConnect()
{
    int lSocket = 0;
    int iRet = 0;
    struct sockaddr_in tServer = {0};


    // Set server info
    tServer.sin_family = AF_INET;
    tServer.sin_port = htons(AVS_CONFIG_SERVER_PORT);
    tServer.sin_addr.s_addr = AVS_CONFIG_SERVER_ADDR;

    // Create a TCP socket
    if ((lSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        DEBUG_PRINTF("avsConnect socket failed!\r\n");
        chip_reboot();
        return lSocket;
    }

    // Connect to server
    if ((iRet = connect(lSocket, (struct sockaddr *) &tServer, sizeof(tServer))) < 0) {
        close(lSocket);
        return -1;
    }

    return lSocket;
}

void avsDisconnect(int lSocket)
{
    close(lSocket);
    shutdown(lSocket, SHUT_RDWR);
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Send Alexa request using file stored in SD card
/////////////////////////////////////////////////////////////////////////////////////////////
int avsSendAlexaRequest(int lSocket, const char* pcFileName)
{
    FIL fHandle;
    int iRet = 0;
    char* pcData = g_pcTxRxBuffer;
    uint32_t ulBytesToTransfer = 0;
    uint32_t ulBytesToProcess = AVS_CONFIG_RXTX_BUFFER_SIZE/2;
    uint32_t ulBytesSent = 0;
    struct timeval tTimeout = {10, 0}; // 10 second timeout


    // Open file given complete file path in SD card
    if (sdcard_open(&fHandle, pcFileName, 0, 1)) {
        DEBUG_PRINTF("avsSendAlexaRequest sdcard_open failed!\r\n");
        return 0;
    }

    // Get file size
    ulBytesToTransfer = sdcard_size(&fHandle);
    if (!ulBytesToTransfer) {
        DEBUG_PRINTF("avsSendAlexaRequest sdcard_size failed!\r\n");
        sdcard_close(&fHandle);
        return 0;
    }
    DEBUG_PRINTF(">> %s %d bytes (16-bit)\r\n", pcFileName, (int)ulBytesToTransfer);
    ulBytesToTransfer /= 2;


    // Set a 10-second timeout for the operation
    setsockopt(lSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tTimeout, sizeof(tTimeout));

    // Negotiate the bytes to transfer
    iRet = send(lSocket, (char*)&ulBytesToTransfer, sizeof(ulBytesToTransfer), 0);
    if (iRet != sizeof(ulBytesToTransfer)) {
        DEBUG_PRINTF("avsSendAlexaRequest send failed! %d %d\r\n\r\n", iRet, sizeof(ulBytesToTransfer));
        sdcard_close(&fHandle);
        return 0;
    }

    // Send the flag for configurations
    // Currently the flag is only for specifying the sampling rate of the response
    uint32_t ulFlag = AVS_CONFIG_SAMPLING_RATE;
    iRet = send(lSocket, (char*)&ulFlag, sizeof(ulFlag), 0);
    if (iRet != sizeof(ulFlag)) {
        DEBUG_PRINTF("avsSendAlexaRequest send failed! %d %d\r\n\r\n", iRet, sizeof(ulFlag));
        sdcard_close(&fHandle);
        return 0;
    }

    //DEBUG_PRINTF(">> Sent %d bytes to: ('%s', %d)\r\n", size_tx, addr, port);


    // Send the total bytes in segments of buffer size
    ulBytesToProcess = AVS_CONFIG_RXTX_BUFFER_SIZE/2;
    ulBytesSent = 0;
    while (ulBytesSent != ulBytesToTransfer) {
        // Compute the transfer size
        if (ulBytesToTransfer-ulBytesSent < ulBytesToProcess) {
            ulBytesToProcess = ulBytesToTransfer-ulBytesSent;
        }

        // Read from SD card
        uint32_t ulReadSize = 0;
        sdcard_read(&fHandle, pcData, ulBytesToProcess*2, (UINT*)&ulReadSize);
        ulBytesToProcess = ulReadSize/2;

        // Convert in-place from 16-bit to 8-bit
        pcm16_to_ulaw(ulBytesToProcess*2, pcData, pcData);

        // Send the converted bytes
        iRet = send(lSocket, pcData, ulBytesToProcess, 0);
        if (iRet <= 0) {
            DEBUG_PRINTF("avsSendAlexaRequest send failed! %d %d\r\n\r\n", (int)ulBytesToProcess, iRet);
            return 0;
        }
        //DEBUG_PRINTF(">> Sent %d bytes\r\n", size_tx);

        // Compute the total bytes sent
        ulBytesSent += iRet;
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
/////////////////////////////////////////////////////////////////////////////////////////////
int avsRecvAlexaResponse(int lSocket, const char* pcFileName)
{
    FIL fHandle;
    int iRet = 0;
    char* pcData = g_pcTxRxBuffer;
    char acTemp[AVS_CONFIG_RXTX_BUFFER_SIZE/2];
    uint32_t ulBytesToReceive = 0;
    uint32_t ulBytesReceived = 0;
    uint32_t ulBytesToProcess = sizeof(acTemp);
    struct timeval tTimeout = {10, 0}; // 10 second timeout


    // Open file given complete file path in SD card
    if (sdcard_open(&fHandle, pcFileName, 1, 0)) {
        DEBUG_PRINTF("avsRecvAlexaResponse sd_open failed!\r\n");
        return 0;
    }

    // Set a 10-second timeout for the operation
    setsockopt(lSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tTimeout, sizeof(tTimeout));


    // Negotiate the bytes to transfer
    iRet = recv(lSocket, (char*)&ulBytesToReceive, sizeof(ulBytesToReceive), 0);
    if (iRet < sizeof(ulBytesToReceive)) {
        DEBUG_PRINTF("avsRecvAlexaResponse recv failed! %d %d\r\n\r\n", iRet, sizeof(ulBytesToReceive));
        sdcard_close(&fHandle);
        return 0;
    }
    DEBUG_PRINTF(">> Total bytes to recv %d (8-bit compressed)\r\n", (int)ulBytesToReceive);


    // Receive the total bytes in segments of buffer size
    while (ulBytesReceived < ulBytesToReceive) {
        // Compute the transfer size
        if (ulBytesToReceive-ulBytesReceived < ulBytesToProcess) {
            ulBytesToProcess = ulBytesToReceive-ulBytesReceived;
        }

        // Receive the bytes of transfer size
        iRet = recv(lSocket, acTemp, ulBytesToProcess, 0);
        if (iRet <= 0) {
            DEBUG_PRINTF("avsRecvAlexaResponse recv failed! %d %d\r\n\r\n", (int)ulBytesToProcess, iRet);
            sdcard_close(&fHandle);
            return 0;
        }
        //DEBUG_PRINTF(">> Recv  %d bytes\r\n", size_rx);

        // Compute the total bytes received
        ulBytesReceived += iRet;

        // Convert 8-bit data to 16-bit data before saving
        ulaw_to_pcm16(iRet, acTemp, pcData);

        // Save to 16-bit decoded data to SD card
        uint32_t ulWriteSize = 0;
        sdcard_write(&fHandle, pcData, iRet*2, (UINT*)&ulWriteSize);
        if (iRet*2 != ulWriteSize) {
            DEBUG_PRINTF("avsRecvAlexaResponse sdcard_write failed! %d %d\r\n\r\n", iRet*2, (int)ulWriteSize);
            sdcard_close(&fHandle);
            return 0;
        }
        //DEBUG_PRINTF(">> Wrote %d bytes to SD card\r\n", size_written);
    }


    // Close the file
    sdcard_close(&fHandle);
    DEBUG_PRINTF(">> %s %d bytes (16-bit)\r\n", pcFileName, (int)ulBytesReceived*2);

    // f_read fails when sd_dir() is not called for some reason
    DEBUG_PRINTF("\r\nChecking SD card directory...\r\n");
    sdcard_dir("");

    return ulBytesReceived*2;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Play audio file from SD card given the complete file path
// - Uses a pre-allocated buffer for faster transfer
// - Reads 1KB segment from file then converts it to stereo (2-channel) audio
//   by duplicating each short then transfers the 2KB segment to I2S Master
/////////////////////////////////////////////////////////////////////////////////////////////
int avsPlayAlexaResponse(const char* pcFileName)
{
    FIL fHandle;
    uint32_t ulFileSize = 0;
    uint32_t ulFileOffset = 0;
    uint32_t ulReadSize = 0;
    uint32_t ulTransferSize = 0;
    char* pcData = g_pcAudioBuffer;
    char acTemp[AVS_CONFIG_AUDIO_BUFFER_SIZE/2];


    // Open file given complete file path in SD card
    if (sdcard_open(&fHandle, pcFileName, 0, 1)) {
        DEBUG_PRINTF("avsPlayAlexaResponse sdcard_open failed!\r\n");
        return 0;
    }

    // Get file size
    ulFileSize = sdcard_size(&fHandle);
    if (!ulFileSize) {
        DEBUG_PRINTF("avsPlayAlexaResponse sdcard_size failed!\r\n");
        sdcard_close(&fHandle);
        return 0;
    }
    DEBUG_PRINTF(">> %s %d bytes (16-bit, %s, mono)\r\n", pcFileName, (int)ulFileSize, getConfigSamplingRateStr());

    do {
        // Process transfer if the speaker FIFO is empty
        if (speaker_ready()) {

            // Compute size to transfer
            ulTransferSize = sizeof(acTemp);
            if (ulFileSize - ulFileOffset < ulTransferSize) {
                ulTransferSize = ulFileSize - ulFileOffset;
            }

#if 1
            // Read data from SD card to buffer
            ulReadSize = 0;
            sdcard_read(&fHandle, acTemp, ulTransferSize, (UINT*)&ulReadSize);
            ulTransferSize = ulReadSize;
#else
            // Read data from SD card to buffer
            ulTotalReadSize = 0;
            do
            {
                ulReadSize = 0;
                f_read(fHandle, &acTemp[ulTotalReadSize], 128, &ulReadSize);
                ulTotalReadSize += ulReadSize;
                ulFileOffset += ulReadSize;
            }
            while (ulTotalReadSize < ulTransferSize);
#endif
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
                speaker_play(pcData, ulTransferSize*2);

                // Increment offset
                ulFileOffset += ulTransferSize;

                // Clear interrupt flag
                i2s_clear_int_flag(MASK_I2S_PEND_FIFO_TX_EMPTY);
            }
        }
    } while (ulFileOffset != ulFileSize);


    // Close the file
    sdcard_close(&fHandle);

    DEBUG_PRINTF(">> Total bytes played %d\r\n", (int)ulFileSize);
    return 1;
}




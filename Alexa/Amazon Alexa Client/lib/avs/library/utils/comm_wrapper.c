/**
  @file comm.c
  @brief
  Communication wrapper module
  Currently supports Ethernet connection only
  Need to support MSTP RS435 connection for PanL
 */
/*
 * ============================================================================
 * History
 * =======
 * 2019-04-02 : Created v1
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

#include <ft900.h>
#include "avs_config.h"    // AVS configuration
#include "lwip/sockets.h"  // lwIP 3rd-party library
#include "comm_wrapper.h"



static int   g_lSocket        = -1;
static int   g_lErr           = 0;



int comm_get_server_port(void)
{
    return AVS_CONFIG_SERVER_PORT;
}

const void* comm_get_server_addr(void)
{
    static struct sockaddr_in tServer;
    tServer.sin_addr.s_addr = AVS_CONFIG_SERVER_ADDR;
    return (ip_addr_t*)&tServer.sin_addr;
}

int comm_err(void)
{
    return g_lErr;
}

int comm_connect(void)
{
    int iRet = 0;
    struct sockaddr_in tServer = {0};


    // Set server info
    tServer.sin_family = AF_INET;
    tServer.sin_port = htons(AVS_CONFIG_SERVER_PORT);
    tServer.sin_addr.s_addr = AVS_CONFIG_SERVER_ADDR;

    // Create a TCP socket
    g_lErr = 0;
    if ((g_lSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        g_lErr = 1;
        return 0;
    }

    // Connect to server
    if ((iRet = connect(g_lSocket, (struct sockaddr *) &tServer, sizeof(tServer))) < 0) {
        close(g_lSocket);
        g_lSocket = -1;
        return 0;
    }

    return 1;
}

void comm_disconnect(void)
{
    if (g_lSocket >= 0) {
        close(g_lSocket);
        shutdown(g_lSocket, SHUT_RDWR);
        g_lSocket = -1;
    }
}

int comm_isconnected(void)
{
    return (g_lSocket>-1);
}

void comm_setsockopt(int lTimeoutSecs, int lIsSend)
{
    struct timeval tTimeout = {lTimeoutSecs, 0};
    if (lIsSend) {
        setsockopt(g_lSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tTimeout, sizeof(tTimeout));
    }
    else {
        setsockopt(g_lSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tTimeout, sizeof(tTimeout));
    }
}

int comm_send(void *pvBuffer, int lSize)
{
    return send(g_lSocket, pvBuffer, lSize, 0);
}

int comm_recv(void *pvBuffer, int lSize)
{
    return recv(g_lSocket, pvBuffer, lSize, 0);
}



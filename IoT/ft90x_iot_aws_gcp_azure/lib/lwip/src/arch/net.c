/**
  @file net.c
  @brief Abstraction file for handling access to lwIP library.
 */
/*
 * ============================================================================
 * History
 * =======
 * 2017-03-15 : Created
 *
 * (C) Copyright Bridgetek Pte Ltd
 * ============================================================================
 *
 * This source code ("the Software") is provided by Bridgetek Pte Ltd
 * ("Bridgetek") subject to the licence terms set out
 * http://www.ftdichip.com/FTSourceCodeLicenceTerms.htm ("the Licence Terms").
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

#include <stdint.h>
#include <string.h>

#include <ft900.h>
#include "net.h"
#ifdef NET_USE_EEPROM
#include <ft900_i2cm.h>
#endif
#include <ft900_eth.h>



//#define NET_DEBUG
#ifdef NET_DEBUG
#include "tinyprintf.h"
#define NET_DEBUG_PRINTF(...) do {tfp_printf(__VA_ARGS__);} while (0)
#else
#define NET_DEBUG_PRINTF(...)
#endif

/* CONSTANTS ***********************************************************************/

/* GLOBAL VARIABLES ****************************************************************/

#if LWIP_NETIF_STATUS_CALLBACK || LWIP_NETIF_LINK_CALLBACK
/**
 @brief Callback function for network available/unavailable signal.
*/
static fn_status_cb gfn_status = NULL;
#endif

/* LOCAL VARIABLES *****************************************************************/

/**
 @brief Default network hostname.
 */
#if LWIP_NETIF_HOSTNAME
static char default_hostname[16] = "FT90x";
#endif

/** @brief Pointer to network interface structure.
 * @details This is used by lwIP to address an interface. We have only one interface
 *         to which this is the handle that is passed to lwIP for all relevant
 *         operations.
 */
static struct netif g_netif;
static uint8_t g_dhcp = 0;

/* MACROS **************************************************************************/

/** @brief Simple minimum macro.
 */
#define MIN(a,b) (a<b)?a:b

/** @brief Sets the name of the network interface.
 */
#define NETIF_SET_NAME(netif,c1,c2) do { (netif)->name[0] = c1; (netif)->name[1] = c2; } while (0)

/* LOCAL FUNCTIONS / INLINES *******************************************************/

#ifdef NET_USE_EEPROM
static int8_t ee_read(uint8_t location, uint8_t *data, const uint16_t len);
static int8_t ee_write(uint8_t location, uint8_t *data, const uint16_t len);
#endif

/* FUNCTIONS ***********************************************************************/

/** @brief Query the link status for the interface.
 *  @details The link status is up when there is a valid connection to another
 *      Ethernet device such as a switch or router. It does not mean that the
 *      network is available.
 *  @returns Non-zero if the link is up.
 */
uint8_t net_is_link_up(void)
{
    return netif_is_link_up(&g_netif);
}

/** @brief Sets the link status for the interface to be up.
 */
void net_set_link_up()
{
    netif_set_link_up(&g_netif);
}

/** @brief Sets the link status for the interface to be down.
 */
void net_set_link_down()
{
    netif_set_link_down(&g_netif);
}

/** @brief Query the network status for the interface.
 *  @returns Non-zero if the network is up and configured.
 */
uint8_t net_is_up()
{
    return netif_is_up(&g_netif);
}

/** @brief Query the status for the network.
 *  @details The status is ready when there is a valid connection to another
 *  Ethernet device such as a switch or router AND a valid IP address is
 *  configured.
 *  @returns Non-zero if the network is ready.
 */
uint8_t net_is_ready(void)
{
    if (netif_is_link_up(&g_netif))
    {
        /* Valid IP address set and link is UP. */
        return (g_netif.ip_addr.addr != IPADDR_ANY);
    }
    return 0;
}

#ifdef NET_USE_EEPROM
int8_t net_update_eeprom(struct eeprom_net_config *setval)
{
    int8_t i2c_status = 0;
    struct eeprom_net_config eepromCur;


    /* Set the I2C Master pins to channel 1 */
    sys_i2c_swop(1);
    i2cm_init(I2CM_NORMAL_SPEED, 10000);

    //read MAC address and other network parameters.
    i2c_status = ee_read(0, (uint8_t *)&eepromCur, sizeof(struct eeprom_net_config));
    if (i2c_status == 0)
    {
        if (memcmp(&eepromCur, setval, sizeof(struct eeprom_net_config)) != 0)
        {
            i2c_status = ee_write(0, (uint8_t *)setval, sizeof(struct eeprom_net_config));
            NET_DEBUG_PRINTF("\nIP config: EEPROM Updated\r\n");
        }
    }
    /* Set the I2C Master pins back to channel 0 */
    i2cm_init(I2CM_NORMAL_SPEED, 100000);
    sys_i2c_swop(0);

    return i2c_status;
}
#endif // NET_USE_EEPROM

#ifdef NET_USE_EEPROM
int8_t net_get_eeprom(struct eeprom_net_config *getval)
{
    int8_t i2c_status = 0;

    /* Set the I2C Master pins to channel 1 */
    sys_i2c_swop(1);
    i2cm_init(I2CM_NORMAL_SPEED, 10000);

    //read MAC address and other network parameters.
    i2c_status = ee_read(0, (uint8_t *)getval, sizeof(struct eeprom_net_config));
    if (i2c_status == 0)
    {
        //NET_DEBUG_PRINTF("\nIP config: EEPROM Read\r\n");
    }
    /* Set the I2C Master pins back to channel 0 */
    i2cm_init(I2CM_NORMAL_SPEED, 100000);
    sys_i2c_swop(0);

    return i2c_status;
}
#endif // NET_USE_EEPROM

#if LWIP_NETIF_STATUS_CALLBACK || LWIP_NETIF_LINK_CALLBACK
/** @brief Display network status for the interface.
 *  @details Prints the network configuration for the
 *      network interface. This is limited to the IP address
 *      gateway and network mask. It is called when the network
 *      status for the interface changes (from down to up or
 *      up to down).
 */
void net_status_cb(struct netif *netif)
{
#ifdef NET_USE_EEPROM
    struct eeprom_net_config netifCur;
#endif // NET_USE_EEPROM

    // DHCP changes received
    NET_DEBUG_PRINTF("\nIP config:\r\n");
    NET_DEBUG_PRINTF("IP %s\r\n", ipaddr_ntoa(&netif->ip_addr));
    NET_DEBUG_PRINTF("GW %s\r\n", ipaddr_ntoa(&netif->gw));
    NET_DEBUG_PRINTF("SU %s\r\n", ipaddr_ntoa(&netif->netmask));

#ifdef NET_USE_EEPROM
    if (netif_is_link_up(netif))
    {
        netifCur.ip = netif->ip_addr;
        netifCur.gw = netif->gw;
        netifCur.mask = netif->netmask;
        netifCur.dhcp = g_dhcp;
        netifCur.key = EEPROM_VALID_KEY;

        net_update_eeprom(&netifCur);
    }
#endif // NET_USE_EEPROM

    if (gfn_status) {
        gfn_status(1, netif_is_link_up(netif));
    }
}
#endif

/** @brief Gets the DHCP status of the interface.
 *  @returns Returns a non-zero if DHCP is enabled.
 */
#if LWIP_DHCP
uint8_t net_get_dhcp()
{
    return g_dhcp;
}

void net_set_dhcp(uint8_t flag)
{
    if (flag)
    {
        NET_DEBUG_PRINTF("DHCP starting...\r\n");
        dhcp_start(&g_netif);
        g_dhcp = 1;
    }
    else
    {
        NET_DEBUG_PRINTF("DHCP stopping.\r\n");
        dhcp_stop(&g_netif);
        g_dhcp = 0;
    }
}
#endif

/** @brief Gets the MAC address of the interface.
 *  @returns Returns a pointer to the 6 octets which form the
 *      MAC address of the interface.
 */
uint8_t *net_get_mac()
{
    return g_netif.hwaddr;
}

/** @brief Gets the currently configured IP address of the interface.
 *  @returns Returns structure which contains the IP address. This is
 *      actually a 32 bit integer which can be expressed as 4 octets
 *      to display a traditional IP address.
 */
ip_addr_t net_get_ip()
{
    return g_netif.ip_addr;
}

/** @brief Gets the currently configured gateway address of the interface.
 *  @returns Returns structure which contains the gateway address. This is
 *      actually a 32 bit integer which can be expressed as 4 octets
 *      to display a traditional IP address.
 */
ip_addr_t net_get_gateway()
{
    return g_netif.gw;
}

/** @brief Gets the currently configured network mask of the interface.
 *  @returns Returns structure which contains the network mask. This is
 *      actually a 32 bit integer which can be expressed as 4 octets
 *      to display a traditional network mask.
 */
ip_addr_t net_get_netmask()
{
    return g_netif.netmask;
}

#if (NO_SYS==0)

#define TASK_CONNECT_STACK_SIZE       (512)           //Task Stack Size
#define TASK_CONNECT_PRIORITY         (1)             //Task Priority
static TaskHandle_t gx_TaskConnect;

void vTaskConnect(void *pvParameters)
{
    NET_DEBUG_PRINTF("Task %s started.\r\n", __FUNCTION__);

    while (1)
    {
        // Check for ethernet disconnection.
        if (net_is_link_up())
        {
            if (!ethernet_is_link_up())
            {
                net_set_link_down();
                NET_DEBUG_PRINTF("Ethernet disconnected.\r\n");
            }
        }
        else
        {
            if (ethernet_is_link_up())
            {
                net_set_link_up();
                NET_DEBUG_PRINTF("Ethernet connected.\r\n");
            }
        }
        for (int i = 0; i < 10; i++)
        {
            net_tick();/* 10 ms delay */
            vTaskDelay(1 * portTICK_PERIOD_MS);
        }
    }

    NET_DEBUG_PRINTF("Task %s finished.\r\n", __FUNCTION__);
}
#endif

/** @brief Initialises the network interface.
 *  @details Configures lwIP to use the network configuration passed in
 *      the parameters. Turns on the hardware for the Ethernet interface
 *      and starts lwIP working.
 *  @param ip IP address to use. If non-zero and dhcp is not set this will
 *      use the IP address in the parameter. If zero then the value in the
 *      EEPROM will be used (DHCP will be enabled or disabled depending on
 *      the parameter).
 *  @param gw IP gateway to use.
 *  @param mask IP network mask to use.
 *  @param dhcp Enable DHCP if non-zero. If an address is passed in the ip
 *      parameter then this will be loaded regardless but may be updated by
 *      a DHCP server.
 *  @returns ERR_OK if the network was initialised successfully.
 */
err_t net_init(ip_addr_t ip,
        ip_addr_t gw, ip_addr_t mask,
        int dhcp, ip_addr_t dns, char *hostname,
        fn_status_cb pfn_status)
{
#ifdef NET_USE_EEPROM
    int8_t i2c_status;
    struct eeprom_net_config er;
#endif

#if LWIP_NETIF_HOSTNAME
    // Implement defaults if required.
    if (hostname != NULL)
    {
        if (*hostname == '\0' && strlen(hostname) < sizeof(default_hostname))
        {
            strncpy(default_hostname, hostname, sizeof(default_hostname));
        }
    }
#endif

#ifdef NET_USE_EEPROM
    /* Set the I2C Master pins to channel 1 */
    sys_i2c_swop(1);
    i2cm_init(I2CM_NORMAL_SPEED, 10000);

    //read MAC address and other network parameters.
    i2c_status = ee_read(0, (uint8_t *)&er, sizeof(struct eeprom_net_config));
    if (i2c_status == 0)
    {
        if (er.key != EEPROM_VALID_KEY && ip.addr != IPADDR_ANY)
        {
            // EEPROM is blank
            // Copy network configuration from EEPROM.
            memcpy(&er.ip, &ip, sizeof(er.ip));
            memcpy(&er.gw, &gw, sizeof(er.gw));
            memcpy(&er.mask, &mask, sizeof(er.mask));
            er.dhcp = (uint8_t)dhcp;
            er.key = EEPROM_VALID_KEY;
            // Write it to the EEPROM
            i2c_status = ee_write(0, (uint8_t *)&er, sizeof(struct eeprom_net_config));
        }
    }

    if (ip.addr == IPADDR_ANY)
    {
#if 0
        if (!dhcp) {
            // Use any pre-existing values from EEPROM.
            memcpy(&ip, &er.ip, sizeof(er.ip));
            memcpy(&gw, &er.gw, sizeof(er.gw));
            memcpy(&mask, &er.mask, sizeof(er.mask));
            dhcp = er.dhcp;
        }
#else
        // Use any pre-existing values from EEPROM.
        memcpy(&ip, &er.ip, sizeof(er.ip));
        memcpy(&gw, &er.gw, sizeof(er.gw));
        memcpy(&mask, &er.mask, sizeof(er.mask));
        dhcp = er.dhcp;
#endif
        if (ip.addr == IPADDR_ANY)
        {
            // Rescue an incorrect IP address.
            dhcp = 1;
        }
    }
#endif // NET_USE_EEPROM

//IPADDR_NONE
    // Read MAC address from pre-programmed area of EEPROM.
    g_netif.hwaddr_len = 6;

#ifdef NET_USE_EEPROM
    i2c_status = ee_read(NET_EEPROM_OFFSET_MACADDRESS, &g_netif.hwaddr[0], 6);

    /* Set the I2C Master pins back to channel 0 */
    i2cm_init(I2CM_NORMAL_SPEED, 100000);
    sys_i2c_swop(0);
#else // NET_USE_EEPROM
    g_netif.hwaddr[0] = 0x44;
    g_netif.hwaddr[1] = 0x6D;
    g_netif.hwaddr[2] = 0x57;
    g_netif.hwaddr[3] = 0xAA;
    g_netif.hwaddr[4] = 0xBB;
    g_netif.hwaddr[5] = 0xCC;
#endif // NET_USE_EEPROM

    NET_DEBUG_PRINTF("MAC = %02x:%02x:%02x:%02x:%02x:%02x\r\n",
            g_netif.hwaddr[0], g_netif.hwaddr[1], g_netif.hwaddr[2],
            g_netif.hwaddr[3], g_netif.hwaddr[4], g_netif.hwaddr[5]);

    NETIF_SET_NAME(&(g_netif),' ','1');

    g_netif.next = NULL;

    // Initialize LWIP
#if defined(NO_SYS) && (NO_SYS==0)
    tcpip_init(NULL, NULL);
#else
    lwip_init();
#endif

    // Add our netif to LWIP (netif_add calls our driver initialization function)
    if (netif_add(&g_netif,
            &ip,
            &mask,
            &gw,
            NULL,
            arch_ft900_init,
            ethernet_input) == NULL)
    {
        NET_DEBUG_PRINTF("netif_add failed\r\n");
        return ERR_IF;
    }

#if LWIP_NETIF_HOSTNAME
    netif_set_hostname(&g_netif, default_hostname);
    NET_DEBUG_PRINTF("Hostname = \"%s\"\r\n", default_hostname);
#endif // LWIP_NETIF_HOSTNAME

#if LWIP_NETIF_STATUS_CALLBACK || LWIP_NETIF_LINK_CALLBACK
    gfn_status = pfn_status;
#endif

    netif_set_default(&g_netif);

#if LWIP_DNS
    ip_addr_t dnsserver = { dns.addr };
    if (dnsserver.addr == 0) {
        ipaddr_aton("8.8.8.8", &dnsserver);
    }
    dns_setserver(0, &dnsserver);
#endif // LWIP_DNS

#if LWIP_NETIF_STATUS_CALLBACK
    netif_set_status_callback(&g_netif, net_status_cb);
#endif

#if LWIP_NETIF_LINK_CALLBACK
    netif_set_link_callback(&g_netif, net_status_cb);
#endif

    netif_set_link_down(&g_netif);
    //netif_set_link_up(&g_netif);
    netif_set_up(&g_netif);

#if defined(NO_SYS) && (NO_SYS==0)
    if (xTaskCreate(vTaskConnect,
            "Connect",
            TASK_CONNECT_STACK_SIZE,
            NULL,
            TASK_CONNECT_PRIORITY,
            &gx_TaskConnect) != pdTRUE) {
        NET_DEBUG_PRINTF("Connect Thread failed\r\n");
    }
#endif

    // Turn on Ethernet receiver.
    arch_ft900_recv_on();

#if LWIP_DHCP
    if (dhcp)
    {
        net_set_dhcp(1);
    }
#endif

    NET_DEBUG_PRINTF("ready\r\n");

    return ERR_OK;
}

/** @brief Processes all lwIP actions for this interface.
 *  @details Calls lwIP architecture specific code to process any
 *      actions that are required for the network interface.
 *      Since this application is not run on an RTOS this needs
 *      to be called often. This does not require to be timer-driven.
 *  @returns Returns value returned by the architecture-specific
 *      action handling function.
 */
err_t net_tick(void)
{
    return arch_ft900_tick(&g_netif);
}

#ifdef NET_USE_EEPROM
/** EEPROM Read routine
 *  @param location The location to start reading
 *  @param data A pointer to the data to read into
 *  @param len The size of data to read */
static int8_t ee_read(uint8_t location, uint8_t *data, const uint16_t len)
{
    return i2cm_read(NET_EEPROM_ADDR, location, data, len);

}

/** EEPROM Write routine
 *  @param location The location to start writing
 *  @param data A pointer to the data to write
 *  @param len The size of data to write */
static int8_t ee_write(uint8_t location, uint8_t *data, const uint16_t len)
{
    uint16_t i;
    int8_t retval = 0;

    /* Transfer a block of data by carrying out a series of byte writes
       and waiting for the write to complete */
    for (i = 0; (i < len) && (retval == 0); ++i)
    {
        retval |= i2cm_write(NET_EEPROM_ADDR, location++, data++, 1);
        delayms(5); // Wait a write cycle time
    }

    return retval;
}

#endif

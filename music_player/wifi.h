/**
 * @file wifi.h
 * Provide wifi connection and network configuration functions
 */

/*********************
 *      INCLUDES
 *********************/
#include <nuttx/config.h>

#if !defined(CONFIG_NETUTILS_DHCPC) || !defined(CONFIG_WIRELESS_WAPI) || !defined(CONFIG_NETUTILS_NTPCLIENT) || (CONFIG_SCHED_LPNTHREADS < 2)
#warning "If you want enable WIFI, please make sure DHCPC, WAPI, IEEE80211, NTP to be enabled \
          and number of lpthreads need greater than or equal to 2."
#define WIFI_ENABLED 0
#else
#define WIFI_ENABLED 1
#endif

#if WIFI_ENABLED

#include <nuttx/wqueue.h>

#include "wireless/wapi.h"

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    char ssid[128];
    char pswd[128];
    enum wpa_ver_e ver_flag;
    clock_t conn_delay;
} wifi_conf_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void wifi_connect(FAR const wifi_conf_t* conf);

#endif /* WIFI_ENABLED */

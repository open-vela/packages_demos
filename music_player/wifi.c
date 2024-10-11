/**
 * @file wifi.c
 * Provide wifi connection and network configuration functions
 */

/*********************
 *      INCLUDES
 *********************/
#include "wifi.h"

#if WIFI_ENABLED

#include <debug.h>
#include <time.h>

#include "netutils/netlib.h"
#include "netutils/ntpclient.h"

/*********************
 *      DEFINES
 *********************/

#define NET_DEVNAME "wlan0"

 /**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    struct work_s work;
    wifi_conf_t conf;
} wifi_work_conf_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

int wifi_conf_set(FAR wifi_conf_t *usr_conf,
                  FAR struct wpa_wconfig_s *wifi_conf)
{
    if(strlen(usr_conf->ssid) == 0) {
        nerr("ERROR: SSID is NULL\n");
        return -EINVAL;
    }

    switch(usr_conf->ver_flag) {
        case WPA_VER_NONE:
            wifi_conf->auth_wpa = IW_AUTH_WPA_VERSION_DISABLED;
            wifi_conf->cipher_mode = IW_AUTH_CIPHER_NONE;
            wifi_conf->alg = WPA_ALG_NONE;
            break;

        case WPA_VER_1:
            wifi_conf->auth_wpa = IW_AUTH_WPA_VERSION_WPA;
            wifi_conf->cipher_mode = IW_AUTH_CIPHER_TKIP;
            wifi_conf->alg = WPA_ALG_TKIP;
            break;

        case WPA_VER_2:
            wifi_conf->auth_wpa = IW_AUTH_WPA_VERSION_WPA2;
            wifi_conf->cipher_mode = IW_AUTH_CIPHER_CCMP;
            wifi_conf->alg = WPA_ALG_CCMP;
            break;

        default:
            nerr("ERROR: unknown WPA version\n");
            return -EINVAL;
    }

    wifi_conf->ifname      = NET_DEVNAME;
    wifi_conf->sta_mode    = IW_MODE_INFRA;
    wifi_conf->ssid        = usr_conf->ssid;
    wifi_conf->passphrase  = usr_conf->pswd;
    wifi_conf->ssidlen     = strlen(usr_conf->ssid);
    wifi_conf->phraselen   = strlen(usr_conf->pswd);
    wifi_conf->bssid       = NULL;
    wifi_conf->freq        = 0;

    return 0;
}

void wifi_connect_worker(FAR void *arg)
{
    FAR wifi_work_conf_t *wifi_conf = arg;
    struct wpa_wconfig_s conf;

    if(wifi_conf_set(&wifi_conf->conf, &conf) < 0) {
        nerr("ERROR: wifi parameter is wrong\n");
        goto fail;
    }

    if(netlib_ifup(NET_DEVNAME) < 0) {
        nerr("ERROR: %s up failed\n", NET_DEVNAME);
        goto fail;
    }

    if(wpa_driver_wext_associate(&conf) < 0) {
        nerr("ERROR: associate failed\n");
        goto fail;
    }

    if(netlib_obtain_ipv4addr(NET_DEVNAME) < 0) {
        nerr("ERROR: network configuration failed\n");
        goto fail;
    }

    ntpc_start();

fail:
    free(wifi_conf);
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void wifi_connect(FAR const wifi_conf_t *conf)
{
    FAR wifi_work_conf_t *wifi_conf = zalloc(sizeof(*wifi_conf));
    if (wifi_conf == NULL) {
        nerr("ERROR: zalloc failed\n");
        return;
    }

    wifi_conf->conf = *conf;

    work_queue(USRWORK, &wifi_conf->work, wifi_connect_worker, wifi_conf, conf->conn_delay);
}

#endif

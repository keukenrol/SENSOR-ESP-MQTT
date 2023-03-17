/* EDIT FOLLOWING ITEMS ========================== */
const char* mqtt_topic = "MQTT_MAIN_TOPIC";
const char* ota_name = mqtt_topic;
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASS";
IPAddress mqtt_server = { 192, 168, 0, 10 };

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 30       /* Time ESP32 will go to sleep for 59 seconds */

// Static IP settings
#define IP_STATIC 1  // comment out if you want to use DHCP
#if defined(IP_STATIC)
IPAddress local_IP(192, 168, 0, 11);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 4, 4);
IPAddress secondaryDNS(8, 8, 8, 8);
#endif
/* END EDIT ITEMS ========================== */
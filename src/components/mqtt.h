#include <WiFi.h>
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID "KELPNET_IOT"
#define WIFI_PASSWORD "GROENEPAARDENSCHOENEN"
#define MQTT_PORT 1883
#define MQTT_HOST IPAddress(192, 168, 3, 100)

static const char* switch_topic = "tft/switch";

static const int JSON_BUFFER_SIZE = 100;

/** Callback for MQTT subscriptions.
 *
 * First parameter is the topic, the second one is the payload.
 */
using mqtt_callback_t = std::function<void(const std::string &, const std::string &)>;
using mqtt_json_callback_t = std::function<void(const std::string &, JsonObject &)>;

/// Callback function typedef for building JsonObjects.
using json_build_t = std::function<void(JsonObject &)>;
/// Callback function typedef for parsing JsonObjects.
using json_parse_t = std::function<void(JsonObject &)>;

/// Build a JSON string with the provided json build function.
const char *build_json(const json_build_t &f, size_t *length);

/// internal struct for MQTT subscriptions.
struct MQTTSubscription {
  std::string topic;
  uint8_t qos;
  mqtt_callback_t callback;
  bool subscribed;
  uint32_t resubscribe_timeout;
};

/// internal struct for MQTT messages.
struct MQTTMessage {
  std::string topic;
  std::string payload;
  uint8_t qos;  ///< QoS. Only for last will testaments.
  bool retain;
};

enum MQTTClientState {
  MQTT_CLIENT_DISCONNECTED = 0,
  MQTT_CLIENT_RESOLVING_ADDRESS,
  MQTT_CLIENT_CONNECTING,
  MQTT_CLIENT_CONNECTED,
};

String get_local_ip();

wl_status_t get_wifi_status();
bool wifi_is_connected();

void connectToMqtt();
void connectToWifi();
void onMqttConnect(bool sessionPresent) ;
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttSubscribe(uint16_t packetId, uint8_t qos);
void onMqttUnsubscribe(uint16_t packetId);
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
void onMqttPublish(uint16_t packetId);


  void mqtt_subscribe(const std::string &topic, mqtt_callback_t callback, uint8_t qos = 0);
  void mqtt_subscribe_json(const std::string &topic, mqtt_json_callback_t callback, uint8_t qos = 0);
  bool mqtt_publish(const MQTTMessage &message);
  bool mqtt_publish(const std::string &topic, const std::string &payload, uint8_t qos = 0, bool retain = false);
  bool mqtt_publish(const std::string &topic, const char *payload, size_t payload_length, uint8_t qos = 0,
               bool retain = false);
  bool mqtt_publish_json(const std::string &topic, const JsonObject &f, uint8_t qos = 0, bool retain = false);

  bool mqtt_subscribe_(const char *topic, uint8_t qos);
  void mqtt_resubscribe_subscription_(MQTTSubscription *sub);
  void mqtt_resubscribe_subscriptions_();

  void mqtt_loop();
  
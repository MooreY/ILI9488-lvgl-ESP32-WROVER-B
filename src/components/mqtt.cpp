
#include "mqtt.h"
#include <Ticker.h>

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

std::vector<MQTTSubscription> mqtt_subscriptions_;

uint32_t mqtt_connect_begin_;
uint32_t mqtt_last_connected_{0};
uint32_t mqtt_reboot_timeout_{300000};
MQTTClientState mqtt_state{MQTT_CLIENT_DISCONNECTED};

MQTTMessage mqtt_last_will_;
MQTTMessage mqtt_birth_message_;
bool mqtt_sent_birth_message_{false};
MQTTMessage mqtt_shutdown_message_;

String get_local_ip(){
    return WiFi.localIP().toString();
}

wl_status_t get_wifi_status() {
    return WiFi.status();
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.disconnect(true);
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  mqttClient.connect();
}

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  connectToMqtt();
}

bool wifi_is_connected() {
    return WiFi.status() == WL_CONNECTED;
}

void onMqttConnect(bool sessionPresent) {
  mqtt_state =  MQTT_CLIENT_DISCONNECTED;
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  uint16_t packetIdSub = mqttClient.subscribe(switch_topic, 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);
  mqttClient.publish(switch_topic, 0, true, "test 1");
  Serial.println("Publishing at QoS 0");
  uint16_t packetIdPub1 = mqttClient.publish(switch_topic, 1, true, "test 2");
  Serial.print("Publishing at QoS 1, packetId: ");
  Serial.println(packetIdPub1);
  uint16_t packetIdPub2 = mqttClient.publish(switch_topic, 2, true, "test 3");
  Serial.print("Publishing at QoS 2, packetId: ");
  Serial.println(packetIdPub2);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (reason == AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT) {
    Serial.println("Bad server fingerprint.");
  }

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  } else {
      connectToWifi();
  }
}

static bool topic_match(const char *message, const char *subscription, bool is_normal, bool past_separator) {
  // Reached end of both strings at the same time, this means we have a successful match
  if (*message == '\0' && *subscription == '\0')
    return true;

  // Either the message or the subscribe are at the end. This means they don't match.
  if (*message == '\0' || *subscription == '\0')
    return false;

  bool do_wildcards = is_normal || past_separator;

  if (*subscription == '+' && do_wildcards) {
    // single level wildcard
    // consume + from subscription
    subscription++;
    // consume everything from message until '/' found or end of string
    while (*message != '\0' && *message != '/') {
      message++;
    }
    // after this, both pointers will point to a '/' or to the end of the string

    return topic_match(message, subscription, is_normal, true);
  }

  if (*subscription == '#' && do_wildcards) {
    // multilevel wildcard - MQTT mandates that this must be at end of subscribe topic
    return true;
  }

  // this handles '/' and normal characters at the same time.
  if (*message != *subscription)
    return false;

  past_separator = past_separator || *subscription == '/';

  // consume characters
  subscription++;
  message++;

  return topic_match(message, subscription, is_normal, past_separator);
}

static bool topic_match(const char *message, const char *subscription) {
  return topic_match(message, subscription, *message != '\0' && *message != '$', false);
}


void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);

  for (auto &subscription : mqtt_subscriptions_)
    if (topic_match(topic, subscription.topic.c_str()))
      subscription.callback(topic, payload);
  }

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}


// Publish
bool mqtt_publish(const std::string &topic, const std::string &payload, uint8_t qos, bool retain) {
  return mqtt_publish(topic, payload.data(), payload.size(), qos, retain);
}

bool mqtt_publish(const std::string &topic, const char *payload, size_t payload_length, uint8_t qos,
                                  bool retain) {
  uint16_t ret = mqttClient.publish(topic.c_str(), qos, retain, payload, payload_length);
  delay(0);
  if (ret == 0 ) {
    delay(0);
    ret = mqttClient.publish(topic.c_str(), qos, retain, payload, payload_length);
    delay(0);
  }

  return ret != 0;
}

bool mqtt_publish(const MQTTMessage &message) {
  return mqtt_publish(message.topic, message.payload, message.qos, message.retain);
}

static char *global_json_build_buffer = nullptr;
static size_t global_json_build_buffer_size = 0;

void reserve_global_json_build_buffer(size_t required_size) {
  if (global_json_build_buffer_size == 0 || global_json_build_buffer_size < required_size) {
    delete[] global_json_build_buffer;
    global_json_build_buffer_size = std::max(required_size, global_json_build_buffer_size * 2);

    size_t remainder = global_json_build_buffer_size % 16U;
    if (remainder != 0)
      global_json_build_buffer_size += 16 - remainder;

    global_json_build_buffer = new char[global_json_build_buffer_size];
  }
}

const char *build_json(const json_build_t &f, size_t *length) {
  StaticJsonBuffer<100> jsonBuffer;
  JsonObject& root  = jsonBuffer.createObject();

  f(root);

  reserve_global_json_build_buffer(jsonBuffer.size());
  size_t bytes_written = root.printTo(global_json_build_buffer, global_json_build_buffer_size);
  char jsonChar[JSON_BUFFER_SIZE];
  if (bytes_written >= JSON_BUFFER_SIZE - 1) {
    reserve_global_json_build_buffer(root.measureLength() + 1);
    bytes_written = root.printTo(jsonChar, JSON_BUFFER_SIZE);
  }

  *length = bytes_written;
  return jsonChar;
}

void parse_json(const std::string &data, const json_parse_t &f) {
    StaticJsonBuffer<100> jsonBuffer;
    JsonObject& root  = jsonBuffer.parseObject(data.c_str());

    if (!root.success()) {
        ESP_LOGW(TAG, "Parsing JSON failed.");
        return;
    }

    f(root);
}

bool mqtt_publish_json(const std::string &topic, const json_build_t &f, uint8_t qos,
                                       bool retain) {
  size_t len;
  const char *message = build_json(f, &len);
  return mqtt_publish(topic, message, len, qos, retain);
}


// Subscribe
bool mqtt_subscribe_(const char *topic, uint8_t qos) {

  uint16_t ret = mqttClient.subscribe(topic, qos);
  yield();

  if (ret != 0) {
    ESP_LOGV(TAG, "subscribe(topic='%s')", topic);
  } else {
    delay(5);
    ESP_LOGV(TAG, "Subscribe failed for topic='%s'. Will retry later.", topic);
  }
  return ret != 0;
}
void mqtt_resubscribe_subscription_(MQTTSubscription *sub) {
  if (sub->subscribed)
    return;

  const uint32_t now = millis();
  bool do_resub = sub->resubscribe_timeout == 0 || now - sub->resubscribe_timeout > 1000;

  if (do_resub) {
    sub->subscribed = mqtt_subscribe_(sub->topic.c_str(), sub->qos);
    sub->resubscribe_timeout = now;
  }
}
void mqtt_resubscribe_subscriptions_() {
  for (auto &subscription : mqtt_subscriptions_) {
    mqtt_resubscribe_subscription_(&subscription);
  }
}

void mqtt_subscribe(const std::string &topic, mqtt_callback_t callback, uint8_t qos) {
  MQTTSubscription subscription{
      .topic = topic,
      .qos = qos,
      .callback = std::move(callback),
      .subscribed = false,
      .resubscribe_timeout = 0,
  };
  mqtt_resubscribe_subscription_(&subscription);
  mqtt_subscriptions_.push_back(subscription);
}

void mqtt_subscribe_json(const std::string &topic, mqtt_json_callback_t callback, uint8_t qos) {
  auto f = [callback](const std::string &topic, const std::string &payload) {
    parse_json(payload, [topic, callback](JsonObject &root) { callback(topic, root); });
  };
  MQTTSubscription subscription{
      .topic = topic,
      .qos = qos,
      .callback = f,
      .subscribed = false,
      .resubscribe_timeout = 0,
  };
  mqtt_resubscribe_subscription_(&subscription);
  mqtt_subscriptions_.push_back(subscription);
}

void mqtt_start_connect_() {
  if (!wifi_is_connected())
    return;

  ESP_LOGI(TAG, "Connecting to MQTT...");
  // Force disconnect first
  connectToMqtt();

  mqtt_state = MQTT_CLIENT_CONNECTING;
  mqtt_connect_begin_ = millis();
}

bool mqtt_is_connected() {
  return mqtt_state == MQTT_CLIENT_CONNECTED && mqttClient.connected();
}

void mqtt_check_connected() {
  if (!mqttClient.connected()) {
    if (millis() - mqtt_connect_begin_ > 60000) {
      mqtt_state = MQTT_CLIENT_DISCONNECTED;
    }
    return;
  }

  mqtt_state = MQTT_CLIENT_CONNECTED;
  mqtt_sent_birth_message_ = false;

  ESP_LOGI(TAG, "MQTT Connected!");
  // MQTT Client needs some time to be fully set up.
  delay(100);

  mqtt_resubscribe_subscriptions_();

}

void mqtt_loop() {
  String reason_s;
  if (!wifi_is_connected()) {
    reason_s = "WiFi disconnected";
  }
    
  const uint32_t now = millis();

//   switch (mqtt_state) {
//     case MQTT_CLIENT_DISCONNECTED:
//       if (now - mqtt_connect_begin_ > 5000) {
//         ;//this->start_dnslookup_();
//         ESP_LOGW(TAG, "MQTT Disconnected: %s.", reason_s);
//       }
//       break;
//     case MQTT_CLIENT_RESOLVING_ADDRESS:
//       ;//this->check_dnslookup_();
//       break;
//     case MQTT_CLIENT_CONNECTING:
//       mqtt_check_connected();
//       break;
//     case MQTT_CLIENT_CONNECTED:
//       if (!mqttClient.connected()) {
//         mqtt_state = MQTT_CLIENT_DISCONNECTED;
//         ESP_LOGW(TAG, "Lost MQTT Client connection!");
//         ;//this->start_dnslookup_();
//       } else {
//         if (!mqtt_birth_message_.topic.empty() && !mqtt_sent_birth_message_) {
//           mqtt_sent_birth_message_ = mqtt_publish(mqtt_birth_message_);
//         }

//         mqtt_last_connected_ = now;
//         mqtt_resubscribe_subscriptions_();
//       }
//       break;
//   }

//   if (millis() - mqtt_last_connected_ > mqtt_reboot_timeout_ && mqtt_reboot_timeout_ != 0) {
//     ESP_LOGE(TAG, "Can't connect to MQTT... Restarting...");
//     //App.reboot();
//   }
}
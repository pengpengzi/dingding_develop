#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <esp_netif.h>
#include <esp_wifi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <Preferences.h>

#ifndef PIN_LED
#define PIN_LED 2
#endif

#ifndef PIN_BUTTON
#define PIN_BUTTON 0
#endif

Preferences preferences;
DNSServer dns;
AsyncWebServer server(80);

String ssid;
String password;
String macAddress;
uint8_t macArray[6];

bool enableDNS = true;

bool lastButtonState = HIGH;
bool currentButtonState = HIGH;
uint32_t pressedTime = 0;

const char *TAG = "MJOLNIR";

esp_netif_t *get_esp_interface_netif(esp_interface_t interface);

void configureInternetlessAP() {
    esp_netif_t *apNetif = get_esp_interface_netif(ESP_IF_WIFI_AP);
    if (apNetif == nullptr) {
        ESP_LOGD(TAG, "AP netif is not ready");
        return;
    }

    esp_err_t stop_err = esp_netif_dhcps_stop(apNetif);
    ESP_LOGD(TAG, "esp_netif_dhcps_stop: %s", esp_err_to_name(stop_err));
    if (stop_err != ESP_OK && stop_err != ESP_ERR_ESP_NETIF_DHCP_ALREADY_STOPPED) {
        return;
    }

    uint8_t noRouterOffer = 0;
    esp_err_t router_err = esp_netif_dhcps_option(
            apNetif,
            ESP_NETIF_OP_SET,
            ESP_NETIF_ROUTER_SOLICITATION_ADDRESS,
            &noRouterOffer,
            sizeof(noRouterOffer));
    ESP_LOGD(TAG, "disable DHCP router offer: %s", esp_err_to_name(router_err));

    esp_netif_dns_info_t dns = {};
    dns.ip.type = ESP_IPADDR_TYPE_V4;
    dns.ip.u_addr.ip4.addr = 0;
    esp_err_t dns_err = esp_netif_set_dns_info(apNetif, ESP_NETIF_DNS_MAIN, &dns);
    ESP_LOGD(TAG, "clear DHCP DNS offer: %s", esp_err_to_name(dns_err));

    esp_err_t start_err = esp_netif_dhcps_start(apNetif);
    ESP_LOGD(TAG, "esp_netif_dhcps_start: %s", esp_err_to_name(start_err));
}

bool convertMacStringToArray() {
    if (macAddress.length() != 17) {
        ESP_LOGD(TAG, "Invalid MAC address length");
        return false;
    }

    unsigned int values[6];
    if (sscanf(macAddress.c_str(), "%02x:%02x:%02x:%02x:%02x:%02x",
               &values[0], &values[1], &values[2],
               &values[3], &values[4], &values[5]) != 6 &&
        sscanf(macAddress.c_str(), "%02x-%02x-%02x-%02x-%02x-%02x",
               &values[0], &values[1], &values[2],
               &values[3], &values[4], &values[5]) != 6) {
        ESP_LOGD(TAG, "Invalid MAC address format");
        return false;
    }

    for (unsigned int value: values) {
        if (value > 0xFF) {
            ESP_LOGD(TAG, "Invalid MAC address byte");
            return false;
        }
    }

    if ((values[0] & 0x01) != 0) {
        ESP_LOGD(TAG, "Invalid MAC address: multicast bit is set");
        return false;
    }

    for (int i = 0; i < 6; i++) {
        macArray[i] = static_cast<uint8_t>(values[i]);
    }

    ESP_LOGD(TAG, "Conversions successfully, MAC: %02x:%02x:%02x:%02x:%02x:%02x",
             macArray[0], macArray[1],
             macArray[2], macArray[3],
             macArray[4], macArray[5]);
    return true;
}

bool createAP() {
    if (!convertMacStringToArray()) {
        digitalWrite(PIN_LED, HIGH);
        return false;
    }

    WiFi.enableAP(true);
    esp_err_t set_mac_err = esp_wifi_set_mac(WIFI_IF_AP, macArray);
    ESP_LOGD(TAG, "esp_wifi_set_mac: %s", esp_err_to_name(set_mac_err));
    if (set_mac_err == ESP_OK) {
        server.end();
        WiFi.softAPdisconnect();
        if (WiFi.softAP(ssid, password)) {
            configureInternetlessAP();
            esp_err_t set_power_err = esp_wifi_set_max_tx_power(34);
            ESP_LOGD(TAG, "esp_wifi_set_max_tx_power: %s", esp_err_to_name(set_power_err));
            digitalWrite(PIN_LED, LOW);
            ESP_LOGD(TAG, "AP created successfully with saved configuration");
            server.begin();
            enableDNS = false;
            return true;
        } else {
            digitalWrite(PIN_LED, HIGH);
            return false;
        }
    } else {
        digitalWrite(PIN_LED, HIGH);
        ESP_LOGD(TAG, "Failed to set MAC address");
        return false;
    }
}

void initWebServer() {
    if (!LittleFS.begin()) {
        ESP_LOGD(TAG, "Failed to initialize LittleFS");
        return;
    }

    server.serveStatic("/", LittleFS, "/");

    server.on("/", HTTP_GET,
              [](AsyncWebServerRequest *request) { request->send(LittleFS, "/index.html", "text/html"); });

    server.onNotFound([](AsyncWebServerRequest *request) { request->redirect("/"); });

    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
        String _ssid_str;
        String _password_str;
        String _mac_str;

        for (int i = 0; i < request->params(); i++) {
            const AsyncWebParameter *p = request->getParam(i);

            if (p->name() == "wifi_ssid") {
                _ssid_str = p->value();
            }

            if (p->name() == "wiif_password") {
                _password_str = p->value();
            }

            if (p->name() == "wifi_mac") {
                _mac_str = p->value();
            }
        }

        if (_ssid_str.length() < 1 || _ssid_str.length() > 63) {
            ESP_LOGD(TAG, "Request send error");
            request->send(200, "text/plain", "SSID format error");
        } else if (_password_str.length() < 8 && _password_str.length() != 0) {
            ESP_LOGD(TAG, "Request send error");
            request->send(200, "text/plain", "Password format error");
        } else if (_mac_str.length() != 17) {
            ESP_LOGD(TAG, "Request send error");
            request->send(200, "text/plain", "MAC length error");
        } else {
            ssid = _ssid_str;
            password = _password_str;
            macAddress = _mac_str;
            if (!createAP()) {
                ESP_LOGD(TAG, "Request send result");
                request->send(200, "text/plain", "创建热点失败，请检查MAC地址是否正确");
            } else {
                preferences.begin("wifi_config", false);
                preferences.putString("ssid", ssid);
                preferences.putString("password", password);
                preferences.putString("mac", macAddress);
                preferences.end();
                // server.end();
                // server.begin();
            }
        }
    });
    server.begin();
}

void setup() {
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    digitalWrite(PIN_LED, HIGH);

    if (!preferences.begin("wifi_config", false)) {
        ESP_LOGD(TAG, "Failed to initialize preferences");
    }

    ssid = preferences.getString("ssid", "");
    password = preferences.getString("password", "");
    macAddress = preferences.getString("mac", "");
    preferences.end();

    WiFi.mode(WIFI_AP);

    if (ssid.length() > 0 && macAddress.length() == 17) {
        if (!createAP()) {
            ESP_LOGD(TAG, "WiFi SSID or MAC error. Creating default AP");
            WiFi.softAP("WIFI MANAGER");
        }
    } else {
        ESP_LOGD(TAG, "No saved WiFi credentials. Creating default AP");
        WiFi.softAP("WIFI MANAGER");
    }

    initWebServer();
    dns.start(53, "*", WiFi.softAPIP());
}

void loop() {
    currentButtonState = digitalRead(PIN_BUTTON);

    if (lastButtonState == HIGH && currentButtonState == LOW) {
        pressedTime = millis();
    } else if (lastButtonState == LOW && currentButtonState == HIGH) {
        if (millis() - pressedTime > 1000) {
            ESP_LOGD(TAG, "Button long pressed, Clean WiFi credentials");
            preferences.begin("wifi_config", false);
            preferences.clear();
            preferences.end();
            delay(1000);
            ESP.restart();
        }
    }
    lastButtonState = currentButtonState;

    if (enableDNS) {
        dns.processNextRequest();
    }
    delay(50);
}

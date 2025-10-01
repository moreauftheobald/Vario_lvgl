#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "global.h"
#include <WiFi.h>
#include <esp_wifi.h>

/**
 * @brief Active WiFi en mode performance pour telechargement
 */
void wifi_enable_performance() {
  if (wifi_connected) {
#ifdef DEBUG_MODE
    ESP_LOGI("WiFi", "Mode performance active");
#endif
    WiFi.setSleep(false);
    esp_wifi_set_ps(WIFI_PS_NONE);
  }
}

/**
 * @brief Repasse WiFi en mode economie
 */
void wifi_enable_power_save() {
  if (wifi_connected) {
#ifdef DEBUG_MODE
    ESP_LOGI("WiFi", "Mode economie active");
#endif
    WiFi.setSleep(true);
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
  }
}

/**
 * @brief Verifie si WiFi toujours connecte
 */
bool wifi_is_connected() {
  return (WiFi.status() == WL_CONNECTED);
}

/**
 * @brief Reconnecte WiFi si deconnecte
 */
bool wifi_reconnect() {
  if (wifi_is_connected()) {
    return true;
  }

#ifdef DEBUG_MODE
  ESP_LOGI("WiFi", "Tentative reconnexion...");
#endif

  // Reactiver WiFi
  WiFi.setSleep(false);
  esp_wifi_set_ps(WIFI_PS_NONE);
  
  // Attendre reconnexion (5s max)
  int attempts = 0;
  while (!wifi_is_connected() && attempts < 10) {
    vTaskDelay(pdMS_TO_TICKS(500));
    attempts++;
  }

  if (wifi_is_connected()) {
#ifdef DEBUG_MODE
    ESP_LOGI("WiFi", "Reconnecte - IP: %s", WiFi.localIP().toString().c_str());
#endif
    return true;
  } else {
#ifdef DEBUG_MODE
    ESP_LOGW("WiFi", "Echec reconnexion");
#endif
    return false;
  }
}

#endif // WIFI_MANAGER_H
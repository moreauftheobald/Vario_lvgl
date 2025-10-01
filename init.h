#ifndef INIT_H
#define INIT_H

#include "global.h"
#include "lang.h"
#include <esp_display_panel.hpp>
#include <lvgl.h>
#include "lvgl_v8_port.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include "metar_task.h"
#include "startup_screen.h"
#include "bmp390_task.h"
#include "ui.h"
#include <esp_task_wdt.h>

using namespace esp_panel::drivers;
using namespace esp_panel::board;

static const char* INIT_TAG = "INIT";

Board* board = nullptr;
LCD* lcd = nullptr;

// =============================================================================
// FONCTIONS D'INITIALISATION
// =============================================================================

/**
 * @brief Initialise les structures de donnees
 */
void init_data_structures() {
#ifdef DEBUG_MODE
  ESP_LOGI(INIT_TAG, "%s", tr(KEY_LOG_DATA_STRUCTURES));
#endif
  add_startup_log(tr(KEY_LOG_DATA_STRUCTURES));
  update_startup_progress(5);

  system_start_time = millis();
  system_initialized = false;
  wifi_connected = false;
  metar_qnh_updated = false;

  memset(&flight_data, 0, sizeof(flight_data_t));
  memset(&metar_data, 0, sizeof(metar_data_t));

  flight_data.qnh_pressure = METAR_QNH_DEFAULT;

#ifdef DEBUG_MODE
  ESP_LOGI(INIT_TAG, "Structures OK");
#endif
  add_startup_log("Structures OK");
  update_startup_progress(10);
}

/**
 * @brief Initialise les buffers circulaires
 */
void init_buffers() {
#ifdef DEBUG_MODE
  ESP_LOGI(INIT_TAG, "%s", tr(KEY_LOG_CIRCULAR_BUFFERS));
#endif
  add_startup_log(tr(KEY_LOG_CIRCULAR_BUFFERS));
  update_startup_progress(15);

  uint32_t psram_caps = MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT;
  uint32_t sram_caps = MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT;

  bool success = true;
  success &= init_rolling_buffer(&vario_history, VARIO_HISTORY_SIZE, sram_caps);
  success &= init_rolling_buffer(&altitude_history, ALTITUDE_HISTORY_SIZE, psram_caps);
  success &= init_rolling_buffer(&speed_history, SPEED_HISTORY_SIZE, psram_caps);
  success &= init_rolling_buffer(&wind_history, WIND_HISTORY_SIZE, psram_caps);

  if (!success) {
#ifdef DEBUG_MODE
    ESP_LOGW(INIT_TAG, "Fallback SRAM pour buffers");
#endif
    success = true;
    success &= init_rolling_buffer(&altitude_history, ALTITUDE_HISTORY_SIZE, sram_caps);
    success &= init_rolling_buffer(&vario_history, VARIO_HISTORY_SIZE, sram_caps);
    success &= init_rolling_buffer(&speed_history, SPEED_HISTORY_SIZE, sram_caps);
    success &= init_rolling_buffer(&wind_history, WIND_HISTORY_SIZE, sram_caps);
  }

  if (success) {
#ifdef DEBUG_MODE
    ESP_LOGI(INIT_TAG, "%s", tr(KEY_STATUS_BUFFERS_OK));
#endif
    add_startup_log(tr(KEY_STATUS_BUFFERS_OK));
  } else {
    add_startup_log("Erreur buffers", true);
  }

  update_startup_progress(20);
}

/**
 * @brief Initialise les primitives FreeRTOS
 */
bool init_freertos_sync() {
#ifdef DEBUG_MODE
  ESP_LOGI(INIT_TAG, "%s", tr(KEY_LOG_FREERTOS_SYNC));
#endif
  add_startup_log(tr(KEY_LOG_FREERTOS_SYNC));
  update_startup_progress(25);

  dataMutex = xSemaphoreCreateMutex();
  sdMutex = xSemaphoreCreateMutex();

  if (dataMutex == NULL || sdMutex == NULL) {
#ifdef DEBUG_MODE
    ESP_LOGE(INIT_TAG, "%s", tr(KEY_ERROR_MUTEX_CREATION));
#endif
    add_startup_log(tr(KEY_ERROR_MUTEX_CREATION), true);
    return false;
  }

  sensorQueue = xQueueCreate(10, sizeof(uint32_t));

  if (sensorQueue == NULL) {
#ifdef DEBUG_MODE
    ESP_LOGE(INIT_TAG, "%s", tr(KEY_ERROR_QUEUE_CREATION));
#endif
    add_startup_log(tr(KEY_ERROR_QUEUE_CREATION), true);
    return false;
  }

#ifdef DEBUG_MODE
  ESP_LOGI(INIT_TAG, "FreeRTOS sync OK");
#endif
  add_startup_log("FreeRTOS sync OK");
  update_startup_progress(30);
  return true;
}

/**
 * @brief Initialise l'ecran et LVGL
 */
bool init_display() {
#ifdef DEBUG_MODE
  ESP_LOGI(INIT_TAG, "Init ecran...");
#endif

  board = new Board();
  if (!board) {
#ifdef DEBUG_MODE
    ESP_LOGE(INIT_TAG, "Erreur allocation Board");
#endif
    return false;
  }

  board->init();
  lcd = board->getLCD();

  if (!lcd) {
#ifdef DEBUG_MODE
    ESP_LOGE(INIT_TAG, "Erreur LCD");
#endif
    return false;
  }

  lcd->configFrameBufferNumber(LVGL_PORT_DISP_BUFFER_NUM);

  auto lcd_bus = lcd->getBus();
  if (lcd_bus && lcd_bus->getBasicAttributes().type == ESP_PANEL_BUS_TYPE_RGB) {
    // Bounce buffer augmenté AVANT WiFi
    static_cast<BusRGB*>(lcd_bus)->configRGB_BounceBufferSize(lcd->getFrameWidth() * 30);
  }

  if (!board->begin()) {
#ifdef DEBUG_MODE
    ESP_LOGE(INIT_TAG, "%s", tr(KEY_ERROR_DISPLAY_INIT));
#endif
    return false;
  }

  if (!lvgl_port_init(board->getLCD(), board->getTouch())) {
#ifdef DEBUG_MODE
    ESP_LOGE(INIT_TAG, "%s", tr(KEY_ERROR_LVGL_INIT));
#endif
    return false;
  }

#ifdef DEBUG_MODE
  ESP_LOGI(INIT_TAG, "Ecran OK");
#endif
  return true;
}

/**
 * @brief Initialise WiFi - Version optimisee
 */
bool init_wifi(const char* ssid, const char* password) {
#ifdef DEBUG_MODE
  ESP_LOGI(INIT_TAG, "Init WiFi: '%s'", ssid);
#endif

  update_startup_status(tr(KEY_WIFI_CONNECTING));
  add_startup_log(tr(KEY_LOG_WIFI_ATTEMPT));
  update_startup_progress(35);

  // Configuration WiFi rapide
  WiFi.mode(WIFI_OFF);
  vTaskDelay(pdMS_TO_TICKS(50));  // 50ms suffisant
  esp_task_wdt_reset();

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(false);  // Pas besoin, on coupe apres METAR
  esp_wifi_set_ps(WIFI_PS_NONE);  // Pas d'economie d'energie = connexion rapide
  
  vTaskDelay(pdMS_TO_TICKS(50));
  esp_task_wdt_reset();

  // Connexion directe
#ifdef DEBUG_MODE
  ESP_LOGI(INIT_TAG, "Connexion...");
#endif

  WiFi.begin(ssid, password);

  // Attente reduite (10 secondes max)
  int attempts = 0;
  const int max_attempts = 10;
  char status_text[64];

  while (WiFi.status() != WL_CONNECTED && attempts < max_attempts) {
    vTaskDelay(pdMS_TO_TICKS(1000));
    attempts++;
    esp_task_wdt_reset();

#ifdef DEBUG_MODE
    if (attempts % 3 == 0) {  // Log toutes les 3 secondes
      ESP_LOGI(INIT_TAG, "Tentative %d/%d - Status: %d", attempts, max_attempts, WiFi.status());
    }
#endif

    // Animation (update plus frequente pour compenser timeout reduit)
    uint8_t dots = (attempts % 4);
    snprintf(status_text, sizeof(status_text), "%s%.*s",
             tr(KEY_WIFI_CONNECTING), dots + 1, "....");
    update_startup_status(status_text);
    
    // Progress bar adaptee au nouveau timeout
    update_startup_progress(35 + (attempts * 3));
  }

  // Resultat
  if (WiFi.status() == WL_CONNECTED) {
    wifi_connected = true;

#ifdef DEBUG_MODE
    ESP_LOGI(INIT_TAG, "WiFi CONNECTE en %d secondes", attempts);
    ESP_LOGI(INIT_TAG, "IP: %s", WiFi.localIP().toString().c_str());
    ESP_LOGI(INIT_TAG, "RSSI: %d dBm", WiFi.RSSI());
#endif

    char log[128];
    snprintf(log, sizeof(log), tr(KEY_LOG_WIFI_CONNECTED),
             WiFi.localIP().toString().c_str());
    add_startup_log(log);
    update_startup_progress(65);

    return true;
  } else {
    wifi_connected = false;

#ifdef DEBUG_MODE
    ESP_LOGW(INIT_TAG, "WiFi ECHEC apres %d secondes - Status: %d", attempts, WiFi.status());
#endif

    add_startup_log(tr(KEY_STATUS_WIFI_OFFLINE_MODE));
    update_startup_progress(65);

    return false;
  }
}

/**
 * @brief Cree les taches
 */
bool create_tasks() {
#ifdef DEBUG_MODE
  ESP_LOGI(INIT_TAG, "%s", tr(KEY_LOG_FREERTOS_TASKS));
#endif
  update_startup_status(tr(KEY_CREATING_TASKS));
  add_startup_log(tr(KEY_LOG_FREERTOS_TASKS));
  update_startup_progress(70);

  // Tache METAR
  BaseType_t result = xTaskCreate(
    metar_task,
    "MetarTask",
    METAR_TASK_STACK,
    NULL,
    METAR_TASK_PRIORITY,
    &metarTaskHandle);

  if (result != pdPASS) {
#ifdef DEBUG_MODE
    ESP_LOGE(INIT_TAG, "%s", tr(KEY_ERROR_METAR_TASK));
#endif
    add_startup_log(tr(KEY_ERROR_METAR_TASK), true);
    return false;
  }

#ifdef DEBUG_MODE
  ESP_LOGI(INIT_TAG, "%s", tr(KEY_LOG_METAR_TASK));
#endif
  add_startup_log(tr(KEY_LOG_METAR_TASK));
  update_startup_progress(75);

  return true;
}

/**
 * @brief Libere buffers
 */
void cleanup_buffers() {
  free_rolling_buffer(&altitude_history);
  free_rolling_buffer(&vario_history);
  free_rolling_buffer(&speed_history);
  free_rolling_buffer(&wind_history);
}

/**
 * @brief Init complete du systeme
 */
bool init_system() {
#ifdef DEBUG_MODE
  ESP_LOGI(INIT_TAG, "=== DEMARRAGE VARIOMETRE ===");
#endif

  // 1. Watchdog
  esp_err_t wdt_err = esp_task_wdt_add(NULL);
  if (wdt_err == ESP_ERR_INVALID_STATE) {
    esp_task_wdt_config_t wdt_config = {
      .timeout_ms = 30000,
      .idle_core_mask = 0,
      .trigger_panic = true
    };
    esp_task_wdt_init(&wdt_config);
    esp_task_wdt_add(NULL);
  }

  // 2. Ecran + LVGL
  if (!init_display()) {
    return false;
  }

  // IMPORTANT: Attendre que LVGL soit vraiment pret
  vTaskDelay(pdMS_TO_TICKS(500));
  esp_task_wdt_reset();

  // 3. MAINTENANT on peut creer l'ecran de demarrage
  create_startup_screen();
  update_startup_status(tr(KEY_INITIALIZATION));
  update_startup_progress(0);

  // 4. Structures
  init_data_structures();
  init_buffers();

  // 5. Sync FreeRTOS
  if (!init_freertos_sync()) {
    cleanup_buffers();
    return false;
  }

  // 6. Capteur BMP390
#ifdef DEBUG_MODE
  ESP_LOGI(INIT_TAG, "Init BMP390...");
#endif
  add_startup_log("Init BMP390");
  update_startup_progress(35);

  if (init_bmp390()) {
    add_startup_log("BMP390 OK");
  } else {
    add_startup_log("BMP390 echec", true);
  }

  // 7. WiFi
  init_wifi(WIFI_SSID, WIFI_PASSWORD);

  // 8. Taches
  if (!create_tasks()) {
    cleanup_buffers();
    return false;
  }

  // 9. Tache BMP390
  if (bmp390_initialized) {
    if (create_bmp390_task()) {
      add_startup_log("Tache BMP390 OK");
    }
  }

  // 10. Attendre METAR (5 secondes max)
  update_startup_status(tr(KEY_METAR_RETRIEVING));
  add_startup_log(tr(KEY_LOG_SEARCHING_METAR));
  update_startup_progress(80);

  uint32_t metar_wait = millis();
  while (!metar_qnh_updated && (millis() - metar_wait) < 5000) {
    esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  // 11. Finalisation
  system_initialized = true;
  update_startup_progress(90);

  if (metar_qnh_updated) {
    char metar_info[128];
    snprintf(metar_info, sizeof(metar_info), tr(KEY_LOG_METAR_RETRIEVED),
             metar_data.station_id, metar_data.qnh_hpa);
    add_startup_log(metar_info);
  } else {
    add_startup_log(tr(KEY_LOG_METAR_UNAVAILABLE));
  }

#ifdef DEBUG_MODE
  ESP_LOGI(INIT_TAG, "=== SYSTEME PRET ===");
  ESP_LOGI(INIT_TAG, "QNH: %.1f hPa (%s)",
           flight_data.qnh_pressure,
           metar_qnh_updated ? "METAR" : "Standard");
#endif

  // 12. Animation fin (3 secondes)
  uint32_t anim_start = millis();
  uint32_t wait_seconds = 3;

  update_startup_status(tr(KEY_SYSTEM_READY));
  update_startup_progress(100);

  lvgl_port_lock(-1);
  lv_obj_set_style_text_color(startup_screen->status_label, lv_color_hex(0x00ff88), 0);
  lvgl_port_unlock();

  add_startup_log(tr(KEY_LOG_STARTUP_SUCCESS));

  // Compte a rebours
  char countdown_text[128];
  while ((millis() - anim_start) < (wait_seconds * 1000)) {
    uint32_t elapsed_s = (millis() - anim_start) / 1000;
    uint32_t remaining_s = wait_seconds - elapsed_s;

    if (remaining_s > 0) {
      snprintf(countdown_text, sizeof(countdown_text),
               tr(KEY_LAUNCHING_INTERFACE), remaining_s);
      update_startup_status(countdown_text);
    }

    esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(200));
  }

  update_startup_status(tr(KEY_MAIN_INTERFACE_LAUNCH));
  esp_task_wdt_reset();
  vTaskDelay(pdMS_TO_TICKS(200));

  // 13. Transition vers interface principale
  esp_task_wdt_reset();
  destroy_startup_screen();

  esp_task_wdt_reset();
  vTaskDelay(pdMS_TO_TICKS(200));

  esp_task_wdt_reset();
  show_main_interface();

  // 14. Retirer watchdog de la loop
  esp_task_wdt_delete(NULL);

  // Couper WiFi pour libérer PSRAM et éviter conflit avec RGB LCD
  if (wifi_connected) {
#ifdef DEBUG_MODE
    ESP_LOGI(INIT_TAG, "WiFi maintenu actif (mode economie pour tiles OSM)");
#endif
    // Mode economie legere (garde connexion mais reduit conso)
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
    WiFi.setSleep(true);
  }

#ifdef DEBUG_MODE
  ESP_LOGI(INIT_TAG, "Init terminee");
#endif

  return true;
}

#endif  // INIT_H
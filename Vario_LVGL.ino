#define CONFIG_I2C_ENABLE_LEGACY_DRIVER 1
#define CONFIG_ESP_WIFI_CACHE_TX_BUFFER_NUM 16
#define CONFIG_SPIRAM_TRY_ALLOCATE_WIFI_LWIP 1

#define DEBUG_MODE (1)
#include <Arduino.h>
#include <esp_log.h>
#include "global.h"
#include "init.h"
#include "metar_task.h"

static const char* MAIN_TAG = "MAIN";

void setup() {
  // Initialisation port serie et logs
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial && millis() < 3000) {
    vTaskDelay(10);  // Attendre que le port serie soit pret
  }
  
#ifdef DEBUG_MODE
  esp_log_level_set("*", ESP_LOG_INFO);
  esp_log_level_set("wifi", ESP_LOG_VERBOSE);  // Logs WiFi detailles
  ESP_LOGI(MAIN_TAG, "Demarrage ESP32-S3 Variometre...");
#else
  esp_log_level_set("*", ESP_LOG_NONE);
#endif

  // Lancement de l'initialisation complete du systeme
  if (!init_system()) {
#ifdef DEBUG_MODE
    ESP_LOGE(MAIN_TAG, "ERREUR CRITIQUE: Initialisation systeme echouee");
#endif
    // Redemarrer apres erreur critique
    vTaskDelay(5000);
    ESP.restart();
  }
  
#ifdef DEBUG_MODE
  ESP_LOGI(MAIN_TAG, "Setup termine avec succes");
#endif
}

void loop() {
  // La loop doit ceder du temps CPU regulierement pour eviter le watchdog
  static uint32_t last_mem_check = 0;
  static uint32_t last_stack_check = 0;
  static uint32_t last_yield = 0;
  
  uint32_t now = millis();
  
  // Yield regulier pour eviter watchdog (toutes les 100ms)
  if (now - last_yield > 100) {
    vTaskDelay(pdMS_TO_TICKS(1));  // Ceder le CPU
    last_yield = now;
  }
  
  // Check memoire toutes les 30 secondes
  if (now - last_mem_check > 30000) {
#ifdef DEBUG_MODE
    ESP_LOGI(MAIN_TAG, "Memoire libre: SRAM=%d, PSRAM=%d, Heap=%d bytes", 
             heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             ESP.getFreeHeap());
#endif
    last_mem_check = now;
  }
  
  // Check stack toutes les 10 secondes
  if (now - last_stack_check > 10000) {
#ifdef DEBUG_MODE
    if (metarTaskHandle != NULL) {
      UBaseType_t stackMETAR = uxTaskGetStackHighWaterMark(metarTaskHandle);
      if (stackMETAR < 512) {
        ESP_LOGW(MAIN_TAG, "Stack METAR faible: %d bytes", stackMETAR);
      }
    }
#endif
    last_stack_check = now;
  }
  
  // Petite pause pour ne pas surcharger
  vTaskDelay(10);
}
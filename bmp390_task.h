#ifndef BMP390_TASK_H
#define BMP390_TASK_H

#include "global.h"
#include <Adafruit_BMP3XX.h>
#include <Wire.h>

// =============================================================================
// VARIABLES GLOBALES BMP390
// =============================================================================

static Adafruit_BMP3XX bmp;
static TwoWire I2C_BMP = TwoWire(1);  // Wire1
static TaskHandle_t bmp390TaskHandle = NULL;
static bool bmp390_initialized = false;

// =============================================================================
// FONCTIONS BMP390
// =============================================================================

/**
 * @brief Initialise le capteur BMP390 sur Wire1
 */
bool init_bmp390() {
#ifdef DEBUG_MODE
  ESP_LOGI("BMP390", "Init sur Wire1 (SDA=%d, SCL=%d)", 
           BMP390_I2C_SDA, BMP390_I2C_SCL);
#endif

  // Initialiser Wire1
  if (!I2C_BMP.begin(BMP390_I2C_SDA, BMP390_I2C_SCL, BMP390_I2C_FREQ)) {
#ifdef DEBUG_MODE
    ESP_LOGE("BMP390", "Erreur init Wire1");
#endif
    return false;
  }

  vTaskDelay(pdMS_TO_TICKS(100));

  // Tentative connexion BMP390 (0x77 par defaut)
  if (!bmp.begin_I2C(0x77, &I2C_BMP)) {
#ifdef DEBUG_MODE
    ESP_LOGW("BMP390", "Essai adresse 0x76...");
#endif
    if (!bmp.begin_I2C(0x76, &I2C_BMP)) {
#ifdef DEBUG_MODE
      ESP_LOGE("BMP390", "BMP390 introuvable");
#endif
      return false;
    }
  }

  // Configuration
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_2X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);

#ifdef DEBUG_MODE
  ESP_LOGI("BMP390", "BMP390 OK");
#endif

  bmp390_initialized = true;
  return true;
}

/**
 * @brief Tache FreeRTOS lecture BMP390
 */
void bmp390_task(void* parameter) {
#ifdef DEBUG_MODE
  ESP_LOGI("BMP390", "Demarrage tache");
#endif

  TickType_t last_wake_time = xTaskGetTickCount();

  while (1) {
    if (bmp390_initialized && bmp.performReading()) {
      
      uint32_t current_time = millis();
      
      float temperature = bmp.temperature;
      float pressure_hpa = bmp.pressure / 100.0f;
      float altitude_qne = bmp.readAltitude(SEALEVELPRESSURE_HPA);
      float altitude_qnh = pressure_to_altitude(pressure_hpa, flight_data.qnh_pressure);
      
      // Mise a jour donnees globales
      if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        flight_data.temperature = temperature;
        flight_data.pressure_hpa = pressure_hpa;
        flight_data.altitude_qne = altitude_qne;
        flight_data.altitude_qnh = altitude_qnh;
        flight_data.current_time = current_time;
        
        xSemaphoreGive(dataMutex);
      }

#ifdef DEBUG_MODE
      static uint32_t last_debug = 0;
      if (current_time - last_debug > 5000) {
        ESP_LOGI("BMP390", "T=%.1fC P=%.1fhPa Alt=%.1fm", 
                 temperature, pressure_hpa, altitude_qnh);
        last_debug = current_time;
      }
#endif
    }

    vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(BMP390_UPDATE_RATE_MS));
  }
}

/**
 * @brief Cree la tache BMP390
 */
bool create_bmp390_task() {
  if (!bmp390_initialized) {
#ifdef DEBUG_MODE
    ESP_LOGE("BMP390", "Capteur non init");
#endif
    return false;
  }

  BaseType_t result = xTaskCreate(
    bmp390_task,
    "BMP390",
    BMP390_TASK_STACK,
    NULL,
    BMP390_TASK_PRIORITY,
    &bmp390TaskHandle);

  if (result != pdPASS) {
#ifdef DEBUG_MODE
    ESP_LOGE("BMP390", "Erreur creation tache");
#endif
    return false;
  }

#ifdef DEBUG_MODE
  ESP_LOGI("BMP390", "Tache creee");
#endif

  return true;
}

/**
 * @brief Arrete la tache BMP390
 */
void stop_bmp390_task() {
  if (bmp390TaskHandle != NULL) {
    vTaskDelete(bmp390TaskHandle);
    bmp390TaskHandle = NULL;
  }
}

#endif  // BMP390_TASK_H
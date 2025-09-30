#ifndef GLOBAL_H
#define GLOBAL_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <esp_heap_caps.h>

// Systeme de traduction
#include "lang.h"

// =============================================================================
// CONFIGURATION WIFI
// =============================================================================

#define WIFI_SSID "NAWAK"           // REMPLACER
#define WIFI_PASSWORD "1234567890"    // REMPLACER
#define WIFI_CONNECT_TIMEOUT 30000      // 30 secondes

// =============================================================================
// CONSTANTES
// =============================================================================

// Configuration serie
#define SERIAL_BAUD_RATE 115200

// Configuration METAR
#define METAR_QNH_DEFAULT 1013.25f
#define METAR_TASK_STACK 8192
#define METAR_TASK_PRIORITY 3

// Tailles buffers circulaires
#define ALTITUDE_HISTORY_SIZE 300
#define VARIO_HISTORY_SIZE 100
#define SPEED_HISTORY_SIZE 100
#define WIND_HISTORY_SIZE 50

// =============================================================================
// STRUCTURES DE DONNEES
// =============================================================================

// Structure METAR
typedef struct {
  char station_id[5];
  char raw_metar[256];  // AJOUTER CETTE LIGNE
  float qnh_hpa;
  float distance_km;
  uint32_t observation_time;
  bool valid;
} metar_data_t;

// Structure donnees de vol
typedef struct {
  float altitude_qne;
  float altitude_qnh;
  float qnh_pressure;
  float temperature;
  float pressure_hpa;
  uint32_t current_time;
} flight_data_t;

// Buffer circulaire
typedef struct {
  float* data;
  uint16_t size;
  uint16_t head;
  uint16_t count;
} rolling_buffer_t;

// =============================================================================
// VARIABLES GLOBALES
// =============================================================================

inline flight_data_t flight_data;
inline metar_data_t metar_data;

inline rolling_buffer_t altitude_history;
inline rolling_buffer_t vario_history;
inline rolling_buffer_t speed_history;
inline rolling_buffer_t wind_history;

inline SemaphoreHandle_t dataMutex = NULL;
inline SemaphoreHandle_t sdMutex = NULL;
inline QueueHandle_t sensorQueue = NULL;
inline TaskHandle_t metarTaskHandle = NULL;

inline uint32_t system_start_time = 0;
inline bool system_initialized = false;
inline bool wifi_connected = false;
inline bool metar_qnh_updated = false;

// =============================================================================
// FONCTIONS UTILITAIRES
// =============================================================================

/**
 * @brief Convertit pression en altitude
 */
inline float pressure_to_altitude(float pressure_hpa, float qnh_hpa) {
  return 44330.0f * (1.0f - pow(pressure_hpa / qnh_hpa, 0.1903f));
}

/**
 * @brief Initialise buffer circulaire
 */
inline bool init_rolling_buffer(rolling_buffer_t* buffer, uint16_t size, uint32_t caps) {
  buffer->data = (float*)heap_caps_malloc(size * sizeof(float), caps);
  if (!buffer->data) return false;
  buffer->size = size;
  buffer->head = 0;
  buffer->count = 0;
  memset(buffer->data, 0, size * sizeof(float));
  return true;
}

/**
 * @brief Libere buffer circulaire
 */
inline void free_rolling_buffer(rolling_buffer_t* buffer) {
  if (buffer->data) {
    free(buffer->data);
    buffer->data = nullptr;
  }
  buffer->size = 0;
  buffer->head = 0;
  buffer->count = 0;
}

/**
 * @brief Ajoute valeur au buffer
 */
inline void add_to_buffer(rolling_buffer_t* buffer, float value) {
  if (!buffer->data) return;
  buffer->data[buffer->head] = value;
  buffer->head = (buffer->head + 1) % buffer->size;
  if (buffer->count < buffer->size) buffer->count++;
}

#endif  // GLOBAL_H
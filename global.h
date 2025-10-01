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

#define WIFI_SSID "NAWAK"
#define WIFI_PASSWORD "1234567890"
#define WIFI_CONNECT_TIMEOUT 30000

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

// Position par defaut : Hayange, Moselle, France
#define DEFAULT_LATITUDE 49.3283
#define DEFAULT_LONGITUDE 6.0627

// API METAR
#define METAR_API_URL "https://aviationweather.gov/api/data/metar"
#define METAR_TIMEOUT_MS 10000

// BMP390
#define BMP390_I2C_SDA 8
#define BMP390_I2C_SCL 9
#define BMP390_I2C_FREQ 400000
#define BMP390_TASK_STACK 4096
#define BMP390_TASK_PRIORITY 4
#define BMP390_UPDATE_RATE_MS 100

#define SEALEVELPRESSURE_HPA (1013.25)

// =============================================================================
// STRUCTURES DE DONNEES
// =============================================================================

// Structure METAR
typedef struct {
  char station_id[5];
  char raw_metar[256];
  float qnh_hpa;
  float distance_km;
  uint32_t observation_time;
  bool valid;
} metar_data_t;

// Structure donnees de vol
typedef struct {
  float altitude_qne;
  float altitude_qnh;
  float altitude_qfe;
  float qnh_pressure;
  float temperature;
  float pressure_hpa;
  float vario_ms;
  float takeoff_altitude;
  bool takeoff_set;
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
// VARIABLES GLOBALES (declarations externes)
// =============================================================================

extern flight_data_t flight_data;
extern metar_data_t metar_data;

extern rolling_buffer_t altitude_history;
extern rolling_buffer_t vario_history;
extern rolling_buffer_t speed_history;
extern rolling_buffer_t wind_history;

extern SemaphoreHandle_t dataMutex;
extern SemaphoreHandle_t sdMutex;
extern QueueHandle_t sensorQueue;
extern TaskHandle_t metarTaskHandle;

extern uint32_t system_start_time;
extern bool system_initialized;
extern bool wifi_connected;
extern bool metar_qnh_updated;

static const char* MAIN_TAG = "MAIN";

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
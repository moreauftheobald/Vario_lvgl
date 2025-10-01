#ifndef BMP390_TASK_H
#define BMP390_TASK_H

#include "global.h"
#include <Adafruit_BMP3XX.h>
#include <Wire.h>

// =============================================================================
// VARIABLES GLOBALES BMP390
// =============================================================================

static Adafruit_BMP3XX bmp;
static TwoWire I2C_BMP = TwoWire(1);
static TaskHandle_t bmp390TaskHandle = NULL;
static bool bmp390_initialized = false;

// Variables pour calcul du vario depuis pression
static float prev_pressure = 0.0f;
static uint32_t prev_time = 0;
static bool first_reading = true;

// Variables pour filtre vario
static float vario_filtered = 0.0f;
static bool vario_filter_initialized = false;

// =============================================================================
// FONCTIONS BMP390
// =============================================================================

/**
 * @brief Filtre exponentiel pour lisser le vario
 */
float filter_vario(float vario_raw) {
  const float alpha = 0.3f;  // Coefficient de filtrage (0 = pas de filtrage, 1 = filtrage max)
  
  if (!vario_filter_initialized) {
    vario_filtered = vario_raw;
    vario_filter_initialized = true;
    return vario_raw;
  }
  
  vario_filtered = alpha * vario_raw + (1.0f - alpha) * vario_filtered;
  return vario_filtered;
}

/**
 * @brief Calcule le vario depuis la pression (plus precis)
 * Formule: dh/dt = -(dP/dt) * (R*T)/(P*g*M)
 * Simplifiee: vario ≈ -44330 * (dP/dt) / P
 */
float calculate_vario_from_pressure(float current_pressure, uint32_t current_time) {
  if (first_reading) {
    prev_pressure = current_pressure;
    prev_time = current_time;
    first_reading = false;
    return 0.0f;
  }

  uint32_t dt = current_time - prev_time;
  if (dt == 0) return 0.0f;

  float dt_sec = dt / 1000.0f;
  
  // Derive de la pression (Pa/s)
  float dP_dt = (current_pressure - prev_pressure) / dt_sec;
  
  // Conversion en vario (m/s)
  // Formule barometrique: vario = -44330 / P * dP/dt
  float vario = -44330.0f * dP_dt / current_pressure;
  
  prev_pressure = current_pressure;
  prev_time = current_time;

  return vario;
}

/**
 * @brief Initialise le capteur BMP390 sur Wire1
 */
bool init_bmp390() {
#ifdef DEBUG_MODE
  ESP_LOGI("BMP390", "Init sur Wire1 (SDA=%d, SCL=%d)", 
           BMP390_I2C_SDA, BMP390_I2C_SCL);
#endif

  if (!I2C_BMP.begin(BMP390_I2C_SDA, BMP390_I2C_SCL, BMP390_I2C_FREQ)) {
#ifdef DEBUG_MODE
    ESP_LOGE("BMP390", "Erreur init Wire1");
#endif
    return false;
  }

  vTaskDelay(pdMS_TO_TICKS(100));

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

  // Configuration amelioree pour reduire le bruit
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_16X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_15);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);

#ifdef DEBUG_MODE
  ESP_LOGI("BMP390", "BMP390 OK");
#endif

  bmp390_initialized = true;
  first_reading = true;
  vario_filter_initialized = false;
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
      float pressure_pa = bmp.pressure;  // En Pascals
      float pressure_hpa = pressure_pa / 100.0f;
      float altitude_qne = bmp.readAltitude(SEALEVELPRESSURE_HPA);
      float altitude_qnh = 0.0f;
      float vario = 0.0f;
      
      if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        
        altitude_qnh = pressure_to_altitude(pressure_hpa, flight_data.qnh_pressure);
        
        // Calculer vario depuis pression (plus precis)
        vario = calculate_vario_from_pressure(pressure_pa, current_time);
        
        // Filtrer le vario BMP
        float vario_smooth = filter_vario(vario);
        
        flight_data.temperature = temperature;
        flight_data.pressure_hpa = pressure_hpa;
        flight_data.altitude_qne = altitude_qne;
        flight_data.altitude_qnh = altitude_qnh;
        flight_data.vario_ms = vario_smooth;
        flight_data.current_time = current_time;
        
        // Calcul QFE (hauteur sol)
        if (flight_data.takeoff_set) {
          flight_data.altitude_qfe = altitude_qnh - flight_data.takeoff_altitude;
        } else {
          flight_data.altitude_qfe = 0.0f;
        }
        
        // Ajout aux buffers circulaires
        add_to_buffer(&altitude_history, altitude_qnh);
        add_to_buffer(&vario_history, vario_smooth);
        
        xSemaphoreGive(dataMutex);
      }

#ifdef DEBUG_MODE
      static uint32_t last_debug = 0;
      if (current_time - last_debug > 5000) {
        ESP_LOGI("BMP390", "T=%.1fC P=%.1fhPa Alt=%.1fm Vario=%.1fm/s",  // CHANGE: %.2f -> %.1f
                 temperature, pressure_hpa, altitude_qnh, vario_smooth);
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

/**
 * @brief Enregistre l'altitude de decollage
 */
void set_takeoff_altitude() {
  if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    flight_data.takeoff_altitude = flight_data.altitude_qnh;
    flight_data.takeoff_set = true;
    xSemaphoreGive(dataMutex);
    
#ifdef DEBUG_MODE
    ESP_LOGI("BMP390", "Altitude decollage: %.1fm", flight_data.takeoff_altitude);
#endif
  }
}

/**
 * @brief Reinitialise l'altitude de decollage
 */
void reset_takeoff_altitude() {
  if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    flight_data.takeoff_altitude = 0.0f;
    flight_data.takeoff_set = false;
    xSemaphoreGive(dataMutex);
    
#ifdef DEBUG_MODE
    ESP_LOGI("BMP390", "Altitude decollage reinit");
#endif
  }
}

#endif  // BMP390_TASK_H
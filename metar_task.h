#ifndef METAR_TASK_H
#define METAR_TASK_H

#include "global.h"
#include "lang.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// =============================================================================
// TACHE METAR
// =============================================================================

void metar_task(void* parameter);
bool fetch_nearest_metar(double latitude, double longitude);
bool parse_metar_qnh(const char* metar_string, float* qnh_hpa);
bool parse_metar_json_api(const char* json_response, metar_data_t* metar_data);
float inhg_to_hpa(float qnh_inhg);
void build_metar_api_url(char* url, int max_len);

/**
 * @brief Tache METAR
 */
void metar_task(void* parameter) {
#ifdef DEBUG_MODE
  ESP_LOGI("METAR", "Demarrage tache");
#endif

  // Attendre WiFi (10s max)
  uint32_t wifi_wait = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - wifi_wait) < 10000) {
#ifdef DEBUG_MODE
    ESP_LOGI("METAR", "Attente WiFi... status=%d", WiFi.status());
#endif
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  if (WiFi.status() != WL_CONNECTED) {
#ifdef DEBUG_MODE
    ESP_LOGW("METAR", "WiFi non connecte - QNH standard");
#endif
    
    // Mode hors ligne - QNH saisonnier
    float qnh_standard = METAR_QNH_DEFAULT;
    
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    int month = timeinfo->tm_mon + 1;

    if (month >= 12 || month <= 2) {
      qnh_standard = 1008.0f;
    } else if (month >= 6 && month <= 8) {
      qnh_standard = 1018.0f;
    } else {
      qnh_standard = 1013.0f;
    }

    memset(&metar_data, 0, sizeof(metar_data_t));
    metar_data.qnh_hpa = qnh_standard;
    strcpy(metar_data.station_id, "STD");
    strcpy(metar_data.raw_metar, "Mode hors ligne - QNH standard");
    metar_data.valid = true;

    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
      flight_data.qnh_pressure = qnh_standard;
      xSemaphoreGive(dataMutex);
    }
    
#ifdef DEBUG_MODE
    ESP_LOGI("METAR", "QNH: %.1f hPa (standard)", qnh_standard);
#endif
    
    metar_qnh_updated = true;
    vTaskDelete(NULL);
    return;
  }

#ifdef DEBUG_MODE
  ESP_LOGI("METAR", "WiFi OK - Recuperation METAR...");
#endif

  // Position par defaut
  double current_lat = DEFAULT_LATITUDE;
  double current_lon = DEFAULT_LONGITUDE;

#ifdef DEBUG_MODE
  ESP_LOGI("METAR", "Position: %.4f, %.4f (Hayange)", current_lat, current_lon);
#endif

  // Tentative METAR
  if (fetch_nearest_metar(current_lat, current_lon)) {
#ifdef DEBUG_MODE
    ESP_LOGI("METAR", "QNH: %.1f hPa (station %s)",
             flight_data.qnh_pressure, metar_data.station_id);
#endif
    metar_qnh_updated = true;
  } else {
#ifdef DEBUG_MODE
    ESP_LOGW("METAR", "Echec METAR - QNH standard");
#endif
    
    // Fallback
    flight_data.qnh_pressure = METAR_QNH_DEFAULT;
    strcpy(metar_data.station_id, "STD");
    strcpy(metar_data.raw_metar, "Echec recuperation METAR");
    metar_qnh_updated = true;
  }

#ifdef DEBUG_MODE
  ESP_LOGI("METAR", "Tache terminee");
#endif
  vTaskDelete(NULL);
}

/**
 * @brief Construit URL API METAR
 */
void build_metar_api_url(char* url, int max_len) {
  // API pour les 3 stations proches de Hayange
  snprintf(url, max_len,
           "%s?ids=LFJL,ELLX,LFST&format=json&hours=3",
           METAR_API_URL);
}

/**
 * @brief Recupere METAR via API
 */
bool fetch_nearest_metar(double latitude, double longitude) {
#ifdef DEBUG_MODE
  ESP_LOGI("METAR", "Requete API METAR...");
#endif

  HTTPClient http;
  char url[256];
  build_metar_api_url(url, sizeof(url));

#ifdef DEBUG_MODE
  ESP_LOGI("METAR", "URL: %s", url);
#endif

  http.begin(url);
  http.setTimeout(METAR_TIMEOUT_MS);
  http.setUserAgent("ESP32-Variometer/1.0");

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    
#ifdef DEBUG_MODE
    ESP_LOGI("METAR", "Reponse recue (%d bytes)", payload.length());
#endif

    bool success = parse_metar_json_api(payload.c_str(), &metar_data);
    
    if (success) {
      if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        flight_data.qnh_pressure = metar_data.qnh_hpa;
        xSemaphoreGive(dataMutex);
      }
      
#ifdef DEBUG_MODE
      ESP_LOGI("METAR", "METAR OK: %s - QNH %.1f hPa",
               metar_data.station_id, metar_data.qnh_hpa);
#endif
    }
    
    http.end();
    return success;
    
  } else {
#ifdef DEBUG_MODE
    ESP_LOGE("METAR", "Erreur HTTP: %d", httpCode);
#endif
    http.end();
    
    // Fallback QNH standard
    float qnh_standard = METAR_QNH_DEFAULT;
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    int month = timeinfo->tm_mon + 1;

    if (month >= 12 || month <= 2) {
      qnh_standard = 1008.0f;
    } else if (month >= 6 && month <= 8) {
      qnh_standard = 1018.0f;
    } else {
      qnh_standard = 1013.0f;
    }

    memset(&metar_data, 0, sizeof(metar_data_t));
    metar_data.qnh_hpa = qnh_standard;
    strcpy(metar_data.station_id, "STD");
    snprintf(metar_data.raw_metar, sizeof(metar_data.raw_metar),
             "Erreur HTTP %d - QNH standard", httpCode);
    metar_data.valid = true;

    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
      flight_data.qnh_pressure = qnh_standard;
      xSemaphoreGive(dataMutex);
    }

    return false;
  }
}

/**
 * @brief Parse JSON METAR
 */
bool parse_metar_json_api(const char* json_response, metar_data_t* metar_data) {
  memset(metar_data, 0, sizeof(metar_data_t));
  metar_data->valid = false;
  metar_data->distance_km = 999999.0f;

#ifdef DEBUG_MODE
  ESP_LOGD("METAR", "Parse JSON...");
#endif

  DynamicJsonDocument doc(8192);
  DeserializationError error = deserializeJson(doc, json_response);

  if (error) {
#ifdef DEBUG_MODE
    ESP_LOGE("METAR", "Erreur JSON: %s", error.c_str());
#endif
    return false;
  }

  if (!doc.is<JsonArray>()) {
#ifdef DEBUG_MODE
    ESP_LOGE("METAR", "Format JSON invalide");
#endif
    return false;
  }

  JsonArray metars = doc.as<JsonArray>();

  for (JsonObject metar : metars) {
    const char* icao_id = metar["icaoId"];
    const char* raw_ob = metar["rawOb"];

    if (!icao_id || !raw_ob) continue;

#ifdef DEBUG_MODE
    ESP_LOGD("METAR", "Station: %s", icao_id);
#endif

    // Distances fixes
    float distance = 999.0f;
    if (strcmp(icao_id, "LFJL") == 0) {
      distance = 15.0f;  // Metz-Nancy (le plus proche)
    } else if (strcmp(icao_id, "ELLX") == 0) {
      distance = 30.0f;  // Luxembourg
    } else if (strcmp(icao_id, "LFST") == 0) {
      distance = 120.0f;  // Strasbourg
    }

    float qnh_hpa;
    if (distance < metar_data->distance_km && parse_metar_qnh(raw_ob, &qnh_hpa)) {
      strncpy(metar_data->station_id, icao_id, 4);
      metar_data->station_id[4] = '\0';
      
      // Copier le METAR brut (limite 256 caracteres)
      strncpy(metar_data->raw_metar, raw_ob, sizeof(metar_data->raw_metar) - 1);
      metar_data->raw_metar[sizeof(metar_data->raw_metar) - 1] = '\0';
      
      metar_data->qnh_hpa = qnh_hpa;
      metar_data->distance_km = distance;
      metar_data->observation_time = millis() / 1000;
      metar_data->valid = true;

#ifdef DEBUG_MODE
      ESP_LOGI("METAR", "Candidat: %s (%.1fkm, QNH=%.1f)",
               icao_id, distance, qnh_hpa);
      ESP_LOGI("METAR", "RAW: %s", raw_ob);
#endif
    }
  }

  return metar_data->valid;
}

/**
 * @brief Parse QNH depuis METAR brut
 */
bool parse_metar_qnh(const char* metar_string, float* qnh_hpa) {
  if (!metar_string || !qnh_hpa) return false;

  const char* pos = metar_string;

  while (*pos) {
    // QNH en hPa : Q1013
    if (*pos == 'Q' && isdigit(pos[1])) {
      int qnh_int;
      if (sscanf(pos + 1, "%4d", &qnh_int) == 1) {
        if (qnh_int >= 950 && qnh_int <= 1050) {
          *qnh_hpa = (float)qnh_int;
#ifdef DEBUG_MODE
          ESP_LOGI("METAR", "QNH trouve: %.1f hPa", *qnh_hpa);
#endif
          return true;
        }
      }
    }

    // QNH en inHg : A2992
    if (*pos == 'A' && isdigit(pos[1])) {
      int qnh_inhg_hundredths;
      if (sscanf(pos + 1, "%4d", &qnh_inhg_hundredths) == 1) {
        if (qnh_inhg_hundredths >= 2800 && qnh_inhg_hundredths <= 3200) {
          float qnh_inhg = qnh_inhg_hundredths / 100.0f;
          *qnh_hpa = inhg_to_hpa(qnh_inhg);
#ifdef DEBUG_MODE
          ESP_LOGI("METAR", "QNH trouve: %.2f inHg -> %.1f hPa", qnh_inhg, *qnh_hpa);
#endif
          return true;
        }
      }
    }

    pos++;
  }

#ifdef DEBUG_MODE
  ESP_LOGW("METAR", "QNH non trouve dans METAR");
#endif
  return false;
}

/**
 * @brief Conversion inHg vers hPa
 */
float inhg_to_hpa(float qnh_inhg) {
  return qnh_inhg * 33.8639f;
}

#endif  // METAR_TASK_H
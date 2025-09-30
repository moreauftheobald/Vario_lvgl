#ifndef LANG_H
#define LANG_H

// =============================================================================
// SYSTÈME DE TRADUCTION MULTI-LANGUES
// =============================================================================

// Énumération des langues supportées
typedef enum {
  LANG_FR = 0,  // Français
  LANG_EN = 1,  // English
  LANG_DE = 2,  // Deutsch
  LANG_COUNT    // Nombre total de langues
} language_t;

// Énumération des clés de traduction
typedef enum {
  // Interface principale
  KEY_VARIOMETER_ACTIVE = 0,
  KEY_INTERFACE_VARIOMETER,
  KEY_OSM_MAP,
  KEY_FLIGHT_WIDGETS,
  KEY_TO_DEVELOP,

  // Écran de démarrage
  KEY_INITIALIZATION,
  KEY_SYSTEM_LOGS,
  KEY_SYSTEM_READY,
  KEY_MAIN_INTERFACE_LAUNCH,
  KEY_STARTUP_COMPLETE,
  KEY_LAUNCHING_INTERFACE,

  // Logs système
  KEY_LOG_DATA_STRUCTURES,
  KEY_LOG_CIRCULAR_BUFFERS,
  KEY_LOG_FREERTOS_SYNC,
  KEY_LOG_GPS_CONFIG,
  KEY_LOG_BMP3XX_INIT,
  KEY_LOG_BNO08X_INIT,
  KEY_LOG_KALMAN_FILTER,
  KEY_LOG_ALL_SENSORS_OK,
  KEY_LOG_WIFI_ATTEMPT,
  KEY_LOG_WIFI_CONNECTED,
  KEY_LOG_WIFI_FAILED_OFFLINE,
  KEY_LOG_FREERTOS_TASKS,
  KEY_LOG_METAR_TASK,
  KEY_LOG_ALL_TASKS_OK,
  KEY_LOG_GPS_TASK_OPTIMIZED,
  KEY_LOG_METAR_RETRIEVED,
  KEY_LOG_METAR_UNAVAILABLE,
  KEY_LOG_SEARCHING_METAR,
  KEY_LOG_STARTUP_SUCCESS,

  // États des composants
  KEY_STATUS_OK,
  KEY_STATUS_ERROR,
  KEY_STATUS_BUFFERS_OK,
  KEY_STATUS_GPS_OK,
  KEY_STATUS_WIFI_CONNECTED_IP,
  KEY_STATUS_WIFI_OFFLINE_MODE,
  KEY_STATUS_METAR_STATION_QNH,
  KEY_STATUS_METAR_STANDARD_QNH,

  // METAR et météo
  KEY_QNH_LABEL,
  KEY_SOURCE_LABEL,
  KEY_POSITION_LABEL,
  KEY_STATION_LABEL,
  KEY_DEFAULT_LOCATION,
  KEY_METAR_RETRIEVED_STATION,
  KEY_GPS_POSITION,

  // Temps et progression
  KEY_STARTUP_COUNTDOWN,
  KEY_SECONDS,
  KEY_WINTER_ADJUSTMENT,
  KEY_SUMMER_ADJUSTMENT,
  KEY_SEASONAL_PRESSURE,

  // Messages d'erreur
  KEY_ERROR_MUTEX_CREATION,
  KEY_ERROR_QUEUE_CREATION,
  KEY_ERROR_DISPLAY_INIT,
  KEY_ERROR_LVGL_INIT,
  KEY_ERROR_METAR_TASK,
  KEY_ERROR_GPS_TASK,
  KEY_ERROR_CRITICAL_INIT,

  // États de connexion
  KEY_WIFI_CONNECTING,
  KEY_WIFI_DOTS,
  KEY_METAR_RETRIEVING,
  KEY_CREATING_TASKS,

  // Interface de vol
  KEY_ALTITUDE_GPS,
  KEY_ALTITUDE_QNE,
  KEY_ALTITUDE_QNH,
  KEY_ALTITUDE_QFE,
  KEY_VARIO_INSTANT,
  KEY_VARIO_INTEGRATED,
  KEY_THERMAL_GAIN,
  KEY_TOTAL_GAIN,
  KEY_SPEED_AIR,
  KEY_SPEED_GPS,
  KEY_SPEED_GROUND,
  KEY_DISTANCE_TAKEOFF,
  KEY_DISTANCE_LINEAR,
  KEY_DISTANCE_CUMULATED,
  KEY_BEARING_TAKEOFF,
  KEY_GLIDE_RATIO,
  KEY_WIND_SPEED,
  KEY_WIND_DIRECTION,
  KEY_G_FORCE,
  KEY_PRESSURE,
  KEY_TEMPERATURE,
  KEY_SATELLITES,
  KEY_LATITUDE,
  KEY_LONGITUDE,
  KEY_FLIGHT_TIME,
  KEY_BATTERY,
  KEY_IN_THERMAL,
  KEY_GPS_FIX,

  // Unités
  KEY_UNIT_METERS,
  KEY_UNIT_MS,
  KEY_UNIT_KMH,
  KEY_UNIT_HPA,
  KEY_UNIT_CELSIUS,
  KEY_UNIT_VOLTS,
  KEY_UNIT_DEGREES,
  KEY_UNIT_SECONDS,
  KEY_UNIT_MINUTES,
  KEY_UNIT_HOURS,

  KEY_COUNT  // Nombre total de clés
} translation_key_t;

// =============================================================================
// VARIABLE GLOBALE ET TABLEAU DE TRADUCTIONS
// =============================================================================

// Variable globale pour la langue actuelle
static language_t current_language = LANG_FR;

// Tableau de traductions [LANGUE][CLÉ]
static const char* translations[LANG_COUNT][KEY_COUNT] = {
  // FRANCAIS [LANG_FR]
  {
    "VARIOMETRE ACTIF",      // KEY_VARIOMETER_ACTIVE
    "Interface variometre",  // KEY_INTERFACE_VARIOMETER
    "Carte OSM",             // KEY_OSM_MAP
    "Widgets de vol",        // KEY_FLIGHT_WIDGETS
    "A developper...",       // KEY_TO_DEVELOP

    "Initialisation...",                                   // KEY_INITIALIZATION
    "=== LOGS SYSTEME ===",                                // KEY_SYSTEM_LOGS
    "Systeme pret !",                                      // KEY_SYSTEM_READY
    "Lancement de l'interface...",                         // KEY_MAIN_INTERFACE_LAUNCH
    "Demarrage termine avec succes",                       // KEY_STARTUP_COMPLETE
    "Lancement interface principale dans %d secondes...",  // KEY_LAUNCHING_INTERFACE

    "Initialisation structures de donnees",         // KEY_LOG_DATA_STRUCTURES
    "Initialisation buffers circulaires",           // KEY_LOG_CIRCULAR_BUFFERS
    "Creation mutex et queues FreeRTOS",            // KEY_LOG_FREERTOS_SYNC
    "GPS configure",                                // KEY_LOG_GPS_CONFIG
    "Initialisation BMP3XX",                        // KEY_LOG_BMP3XX_INIT
    "Initialisation BNO08x",                        // KEY_LOG_BNO08X_INIT
    "Filtre de Kalman OK",                          // KEY_LOG_KALMAN_FILTER
    "Tous les capteurs OK",                         // KEY_LOG_ALL_SENSORS_OK
    "Tentative connexion WiFi",                     // KEY_LOG_WIFI_ATTEMPT
    "WiFi connecte - IP: %s",                       // KEY_LOG_WIFI_CONNECTED
    "WiFi echec - Mode hors ligne",                 // KEY_LOG_WIFI_FAILED_OFFLINE
    "Creation taches FreeRTOS",                     // KEY_LOG_FREERTOS_TASKS
    "Tache METAR OK",                               // KEY_LOG_METAR_TASK
    "Toutes les taches OK",                         // KEY_LOG_ALL_TASKS_OK
    "Tache GPS optimisee creee",                    // KEY_LOG_GPS_TASK_OPTIMIZED
    "METAR recupere - Station: %s, QNH: %.1f hPa",  // KEY_LOG_METAR_RETRIEVED
    "METAR non disponible - QNH standard",          // KEY_LOG_METAR_UNAVAILABLE
    "Recherche station METAR proche",               // KEY_LOG_SEARCHING_METAR
    "Demarrage termine avec succes",                // KEY_LOG_STARTUP_SUCCESS

    "[OK]  %s",                                     // KEY_STATUS_OK
    "[ERR] %s",                                     // KEY_STATUS_ERROR
    "Buffers OK",                                   // KEY_STATUS_BUFFERS_OK
    "Structures OK",                                // KEY_STATUS_GPS_OK
    "WiFi connecte - IP: %s",                       // KEY_STATUS_WIFI_CONNECTED_IP
    "WiFi echec - Mode hors ligne",                 // KEY_STATUS_WIFI_OFFLINE_MODE
    "METAR recupere - Station: %s, QNH: %.1f hPa",  // KEY_STATUS_METAR_STATION_QNH
    "METAR non disponible - QNH standard",          // KEY_STATUS_METAR_STANDARD_QNH

    "QNH: %.1f hPa",   // KEY_QNH_LABEL
    "Source: %s",      // KEY_SOURCE_LABEL
    "Position: %s",    // KEY_POSITION_LABEL
    "Station",         // KEY_STATION_LABEL
    "Hayange (57)",    // KEY_DEFAULT_LOCATION
    "METAR recupere",  // KEY_METAR_RETRIEVED_STATION
    "GPS",             // KEY_GPS_POSITION

    "Demarrage interface principale dans %d secondes...",  // KEY_STARTUP_COUNTDOWN
    "secondes",                                            // KEY_SECONDS
    "Ajustement hivernal applique",                        // KEY_WINTER_ADJUSTMENT
    "Ajustement estival applique",                         // KEY_SUMMER_ADJUSTMENT
    "Pression standard de saison appliquee",               // KEY_SEASONAL_PRESSURE

    "Erreur creation mutex",                            // KEY_ERROR_MUTEX_CREATION
    "Erreur creation queues",                           // KEY_ERROR_QUEUE_CREATION
    "Erreur initialisation ecran",                      // KEY_ERROR_DISPLAY_INIT
    "Erreur initialisation LVGL",                       // KEY_ERROR_LVGL_INIT
    "Erreur tache METAR",                               // KEY_ERROR_METAR_TASK
    "Erreur tache GPS",                                 // KEY_ERROR_GPS_TASK,
    "ERREUR CRITIQUE: Initialisation systeme echouee",  // KEY_ERROR_CRITICAL_INIT

    "Connexion WiFi",          // KEY_WIFI_CONNECTING
    "....",                    // KEY_WIFI_DOTS
    "Recuperation METAR...",   // KEY_METAR_RETRIEVING
    "Creation des taches...",  // KEY_CREATING_TASKS

    "Alt GPS",       // KEY_ALTITUDE_GPS
    "Alt QNE",       // KEY_ALTITUDE_QNE
    "Alt QNH",       // KEY_ALTITUDE_QNH
    "Hauteur",       // KEY_ALTITUDE_QFE
    "Vario",         // KEY_VARIO_INSTANT
    "Vario Int",     // KEY_VARIO_INTEGRATED
    "Gain Therm",    // KEY_THERMAL_GAIN
    "Gain Total",    // KEY_TOTAL_GAIN
    "Vitesse Air",   // KEY_SPEED_AIR
    "Vitesse GPS",   // KEY_SPEED_GPS
    "Vitesse Sol",   // KEY_SPEED_GROUND
    "Dist Deco",     // KEY_DISTANCE_TAKEOFF
    "Dist Lin",      // KEY_DISTANCE_LINEAR
    "Dist Cum",      // KEY_DISTANCE_CUMULATED
    "Cap Deco",      // KEY_BEARING_TAKEOFF
    "Finesse",       // KEY_GLIDE_RATIO
    "Vent Vit",      // KEY_WIND_SPEED
    "Vent Dir",      // KEY_WIND_DIRECTION
    "G-metre",       // KEY_G_FORCE
    "Pression",      // KEY_PRESSURE
    "Temperature",   // KEY_TEMPERATURE
    "Satellites",    // KEY_SATELLITES
    "Latitude",      // KEY_LATITUDE
    "Longitude",     // KEY_LONGITUDE
    "Temps Vol",     // KEY_FLIGHT_TIME
    "Batterie",      // KEY_BATTERY
    "En thermique",  // KEY_IN_THERMAL
    "Fix GPS",       // KEY_GPS_FIX

    "m",     // KEY_UNIT_METERS
    "m/s",   // KEY_UNIT_MS
    "km/h",  // KEY_UNIT_KMH
    "hPa",   // KEY_UNIT_HPA
    "C",     // KEY_UNIT_CELSIUS
    "V",     // KEY_UNIT_VOLTS
    "deg",   // KEY_UNIT_DEGREES
    "s",     // KEY_UNIT_SECONDS
    "min",   // KEY_UNIT_MINUTES
    "h"      // KEY_UNIT_HOURS
  },

  // ENGLISH [LANG_EN]
  {
    "VARIOMETER ACTIVE",     // KEY_VARIOMETER_ACTIVE
    "Variometer interface",  // KEY_INTERFACE_VARIOMETER
    "OSM Map",               // KEY_OSM_MAP
    "Flight widgets",        // KEY_FLIGHT_WIDGETS
    "To develop...",         // KEY_TO_DEVELOP

    "Initialization...",                          // KEY_INITIALIZATION
    "=== SYSTEM LOGS ===",                        // KEY_SYSTEM_LOGS
    "System ready!",                              // KEY_SYSTEM_READY
    "Launching interface...",                     // KEY_MAIN_INTERFACE_LAUNCH
    "Startup completed successfully",             // KEY_STARTUP_COMPLETE
    "Launching main interface in %d seconds...",  // KEY_LAUNCHING_INTERFACE

    "Data structures initialization",                // KEY_LOG_DATA_STRUCTURES
    "Circular buffers initialization",               // KEY_LOG_CIRCULAR_BUFFERS
    "FreeRTOS mutex and queues creation",            // KEY_LOG_FREERTOS_SYNC
    "GPS configured",                                // KEY_LOG_GPS_CONFIG
    "BMP3XX initialization",                         // KEY_LOG_BMP3XX_INIT
    "BNO08x initialization",                         // KEY_LOG_BNO08X_INIT
    "Kalman filter OK",                              // KEY_LOG_KALMAN_FILTER
    "All sensors OK",                                // KEY_LOG_ALL_SENSORS_OK
    "WiFi connection attempt",                       // KEY_LOG_WIFI_ATTEMPT
    "WiFi connected - IP: %s",                       // KEY_LOG_WIFI_CONNECTED
    "WiFi failed - Offline mode",                    // KEY_LOG_WIFI_FAILED_OFFLINE
    "FreeRTOS tasks creation",                       // KEY_LOG_FREERTOS_TASKS
    "METAR task OK",                                 // KEY_LOG_METAR_TASK
    "All tasks OK",                                  // KEY_LOG_ALL_TASKS_OK
    "Optimized GPS task created",                    // KEY_LOG_GPS_TASK_OPTIMIZED
    "METAR retrieved - Station: %s, QNH: %.1f hPa",  // KEY_LOG_METAR_RETRIEVED
    "METAR unavailable - Standard QNH",              // KEY_LOG_METAR_UNAVAILABLE
    "Searching nearby METAR station",                // KEY_LOG_SEARCHING_METAR
    "Startup completed successfully",                // KEY_LOG_STARTUP_SUCCESS

    "[OK]  %s",                                      // KEY_STATUS_OK
    "[ERR] %s",                                      // KEY_STATUS_ERROR
    "Buffers OK",                                    // KEY_STATUS_BUFFERS_OK
    "Structures OK",                                 // KEY_STATUS_GPS_OK
    "WiFi connected - IP: %s",                       // KEY_STATUS_WIFI_CONNECTED_IP
    "WiFi failed - Offline mode",                    // KEY_STATUS_WIFI_OFFLINE_MODE
    "METAR retrieved - Station: %s, QNH: %.1f hPa",  // KEY_STATUS_METAR_STATION_QNH
    "METAR unavailable - Standard QNH",              // KEY_STATUS_METAR_STANDARD_QNH

    "QNH: %.1f hPa",    // KEY_QNH_LABEL
    "Source: %s",       // KEY_SOURCE_LABEL
    "Position: %s",     // KEY_POSITION_LABEL
    "Station",          // KEY_STATION_LABEL
    "Hayange (57)",     // KEY_DEFAULT_LOCATION
    "METAR retrieved",  // KEY_METAR_RETRIEVED_STATION
    "GPS",              // KEY_GPS_POSITION

    "Launching main interface in %d seconds...",  // KEY_STARTUP_COUNTDOWN
    "seconds",                                    // KEY_SECONDS
    "Winter adjustment applied",                  // KEY_WINTER_ADJUSTMENT
    "Summer adjustment applied",                  // KEY_SUMMER_ADJUSTMENT
    "Seasonal standard pressure applied",         // KEY_SEASONAL_PRESSURE

    "Mutex creation error",                          // KEY_ERROR_MUTEX_CREATION
    "Queue creation error",                          // KEY_ERROR_QUEUE_CREATION
    "Display initialization error",                  // KEY_ERROR_DISPLAY_INIT
    "LVGL initialization error",                     // KEY_ERROR_LVGL_INIT
    "METAR task error",                              // KEY_ERROR_METAR_TASK
    "GPS task error"                                 // KEY_ERROR_GPS_TASK,
    "CRITICAL ERROR: System initialization failed",  // KEY_ERROR_CRITICAL_INIT

    "WiFi connecting",     // KEY_WIFI_CONNECTING
    "....",                // KEY_WIFI_DOTS
    "METAR retrieval...",  // KEY_METAR_RETRIEVING
    "Creating tasks...",   // KEY_CREATING_TASKS

    "GPS Alt",       // KEY_ALTITUDE_GPS
    "QNE Alt",       // KEY_ALTITUDE_QNE
    "QNH Alt",       // KEY_ALTITUDE_QNH
    "Height",        // KEY_ALTITUDE_QFE
    "Vario",         // KEY_VARIO_INSTANT
    "Vario Int",     // KEY_VARIO_INTEGRATED
    "Thermal Gain",  // KEY_THERMAL_GAIN
    "Total Gain",    // KEY_TOTAL_GAIN
    "Air Speed",     // KEY_SPEED_AIR
    "GPS Speed",     // KEY_SPEED_GPS
    "Ground Speed",  // KEY_SPEED_GROUND
    "Takeoff Dist",  // KEY_DISTANCE_TAKEOFF
    "Linear Dist",   // KEY_DISTANCE_LINEAR
    "Cumul Dist",    // KEY_DISTANCE_CUMULATED
    "Takeoff Brg",   // KEY_BEARING_TAKEOFF
    "Glide Ratio",   // KEY_GLIDE_RATIO
    "Wind Speed",    // KEY_WIND_SPEED
    "Wind Dir",      // KEY_WIND_DIRECTION
    "G-meter",       // KEY_G_FORCE
    "Pressure",      // KEY_PRESSURE
    "Temperature",   // KEY_TEMPERATURE
    "Satellites",    // KEY_SATELLITES
    "Latitude",      // KEY_LATITUDE
    "Longitude",     // KEY_LONGITUDE
    "Flight Time",   // KEY_FLIGHT_TIME
    "Battery",       // KEY_BATTERY
    "In thermal",    // KEY_IN_THERMAL
    "GPS Fix",       // KEY_GPS_FIX

    "m",     // KEY_UNIT_METERS
    "m/s",   // KEY_UNIT_MS
    "km/h",  // KEY_UNIT_KMH
    "hPa",   // KEY_UNIT_HPA
    "C",     // KEY_UNIT_CELSIUS
    "V",     // KEY_UNIT_VOLTS
    "deg",   // KEY_UNIT_DEGREES
    "s",     // KEY_UNIT_SECONDS
    "min",   // KEY_UNIT_MINUTES
    "h"      // KEY_UNIT_HOURS
  },

  // DEUTSCH [LANG_DE]
  {
    "VARIOMETER AKTIV",      // KEY_VARIOMETER_ACTIVE
    "Variometer Interface",  // KEY_INTERFACE_VARIOMETER
    "OSM Karte",             // KEY_OSM_MAP
    "Flug Widgets",          // KEY_FLIGHT_WIDGETS
    "Zu entwickeln...",      // KEY_TO_DEVELOP

    "Initialisierung...",                               // KEY_INITIALIZATION
    "=== SYSTEM LOGS ===",                              // KEY_SYSTEM_LOGS
    "System bereit!",                                   // KEY_SYSTEM_READY
    "Interface wird gestartet...",                      // KEY_MAIN_INTERFACE_LAUNCH
    "Start erfolgreich abgeschlossen",                  // KEY_STARTUP_COMPLETE
    "Hauptinterface wird in %d Sekunden gestartet...",  // KEY_LAUNCHING_INTERFACE

    "Datenstrukturen Initialisierung",               // KEY_LOG_DATA_STRUCTURES
    "Ringpuffer Initialisierung",                    // KEY_LOG_CIRCULAR_BUFFERS
    "FreeRTOS Mutex und Queues Erstellung",          // KEY_LOG_FREERTOS_SYNC
    "GPS konfiguriert",                              // KEY_LOG_GPS_CONFIG
    "BMP3XX Initialisierung",                        // KEY_LOG_BMP3XX_INIT
    "BNO08x Initialisierung",                        // KEY_LOG_BNO08X_INIT
    "Kalman Filter OK",                              // KEY_LOG_KALMAN_FILTER
    "Alle Sensoren OK",                              // KEY_LOG_ALL_SENSORS_OK
    "WiFi Verbindungsversuch",                       // KEY_LOG_WIFI_ATTEMPT
    "WiFi verbunden - IP: %s",                       // KEY_LOG_WIFI_CONNECTED
    "WiFi Fehler - Offline Modus",                   // KEY_LOG_WIFI_FAILED_OFFLINE
    "FreeRTOS Tasks Erstellung",                     // KEY_LOG_FREERTOS_TASKS
    "METAR Task OK",                                 // KEY_LOG_METAR_TASK
    "Alle Tasks OK",                                 // KEY_LOG_ALL_TASKS_OK
    "Optimierte GPS-Aufgabe erstellt",               // KEY_LOG_GPS_TASK_OPTIMIZED
    "METAR abgerufen - Station: %s, QNH: %.1f hPa",  // KEY_LOG_METAR_RETRIEVED
    "METAR nicht verfuegbar - Standard QNH",         // KEY_LOG_METAR_UNAVAILABLE
    "Suche nahe METAR Station",                      // KEY_LOG_SEARCHING_METAR
    "Start erfolgreich abgeschlossen",               // KEY_LOG_STARTUP_SUCCESS

    "[OK]  %s",                                      // KEY_STATUS_OK
    "[ERR] %s",                                      // KEY_STATUS_ERROR
    "Puffer OK",                                     // KEY_STATUS_BUFFERS_OK
    "Strukturen OK",                                 // KEY_STATUS_GPS_OK
    "WiFi verbunden - IP: %s",                       // KEY_STATUS_WIFI_CONNECTED_IP
    "WiFi Fehler - Offline Modus",                   // KEY_STATUS_WIFI_OFFLINE_MODE
    "METAR abgerufen - Station: %s, QNH: %.1f hPa",  // KEY_STATUS_METAR_STATION_QNH
    "METAR nicht verfuegbar - Standard QNH",         // KEY_STATUS_METAR_STANDARD_QNH

    "QNH: %.1f hPa",    // KEY_QNH_LABEL
    "Quelle: %s",       // KEY_SOURCE_LABEL
    "Position: %s",     // KEY_POSITION_LABEL
    "Station",          // KEY_STATION_LABEL
    "Hayange (57)",     // KEY_DEFAULT_LOCATION
    "METAR abgerufen",  // KEY_METAR_RETRIEVED_STATION
    "GPS",              // KEY_GPS_POSITION

    "Hauptinterface wird in %d Sekunden gestartet...",  // KEY_STARTUP_COUNTDOWN
    "Sekunden",                                         // KEY_SECONDS
    "Winter Anpassung angewandt",                       // KEY_WINTER_ADJUSTMENT
    "Sommer Anpassung angewandt",                       // KEY_SUMMER_ADJUSTMENT
    "Saison Standard Druck angewandt",                  // KEY_SEASONAL_PRESSURE

    "Mutex Erstellungsfehler",                                   // KEY_ERROR_MUTEX_CREATION
    "Queue Erstellungsfehler",                                   // KEY_ERROR_QUEUE_CREATION
    "Display Initialisierungsfehler",                            // KEY_ERROR_DISPLAY_INIT
    "LVGL Initialisierungsfehler",                               // KEY_ERROR_LVGL_INIT
    "METAR Task Fehler",                                         // KEY_ERROR_METAR_TASK
    "GPS-Aufgabe Fehler",                                        // KEY_ERROR_GPS_TASK,
    "KRITISCHER FEHLER: System Initialisierung fehlgeschlagen",  // KEY_ERROR_CRITICAL_INIT

    "WiFi Verbindung",           // KEY_WIFI_CONNECTING
    "....",                      // KEY_WIFI_DOTS
    "METAR Abruf...",            // KEY_METAR_RETRIEVING
    "Tasks werden erstellt...",  // KEY_CREATING_TASKS

    "GPS Höhe",        // KEY_ALTITUDE_GPS
    "QNE Höhe",        // KEY_ALTITUDE_QNE
    "QNH Höhe",        // KEY_ALTITUDE_QNH
    "Höhe",            // KEY_ALTITUDE_QFE
    "Vario",           // KEY_VARIO_INSTANT
    "Vario Int",       // KEY_VARIO_INTEGRATED
    "Thermik Gewinn",  // KEY_THERMAL_GAIN
    "Gesamt Gewinn",   // KEY_TOTAL_GAIN
    "Luftgeschw",      // KEY_SPEED_AIR
    "GPS Geschw",      // KEY_SPEED_GPS
    "Bodengeschw",     // KEY_SPEED_GROUND
    "Start Entf",      // KEY_DISTANCE_TAKEOFF
    "Linear Entf",     // KEY_DISTANCE_LINEAR
    "Kum Entf",        // KEY_DISTANCE_CUMULATED
    "Start Kurs",      // KEY_BEARING_TAKEOFF
    "Gleitzahl",       // KEY_GLIDE_RATIO
    "Wind Geschw",     // KEY_WIND_SPEED
    "Wind Rich",       // KEY_WIND_DIRECTION
    "G-Messer",        // KEY_G_FORCE
    "Druck",           // KEY_PRESSURE
    "Temperatur",      // KEY_TEMPERATURE
    "Satelliten",      // KEY_SATELLITES
    "Breitengrad",     // KEY_LATITUDE
    "Längengrad",      // KEY_LONGITUDE
    "Flugzeit",        // KEY_FLIGHT_TIME
    "Batterie",        // KEY_BATTERY
    "In Thermik",      // KEY_IN_THERMAL
    "GPS Fix",         // KEY_GPS_FIX

    "m",     // KEY_UNIT_METERS
    "m/s",   // KEY_UNIT_MS
    "km/h",  // KEY_UNIT_KMH
    "hPa",   // KEY_UNIT_HPA
    "C",     // KEY_UNIT_CELSIUS
    "V",     // KEY_UNIT_VOLTS
    "deg",   // KEY_UNIT_DEGREES
    "s",     // KEY_UNIT_SECONDS
    "min",   // KEY_UNIT_MINUTES
    "h"      // KEY_UNIT_HOURS
  }
};

// =============================================================================
// FONCTIONS DE TRADUCTION
// =============================================================================

/**
 * @brief Définit la langue actuelle
 * @param lang Langue à utiliser
 */
static inline void set_language(language_t lang) {
  if (lang < LANG_COUNT) {
    current_language = lang;
  }
}

/**
 * @brief Récupère la langue actuelle
 * @return Langue actuelle
 */
static inline language_t get_language(void) {
  return current_language;
}

/**
 * @brief Récupère une chaîne traduite
 * @param key Clé de traduction
 * @return Chaîne traduite dans la langue actuelle
 */
static inline const char* tr(translation_key_t key) {
  if (key >= KEY_COUNT || current_language >= LANG_COUNT) {
    return "INVALID_KEY";
  }
  return translations[current_language][key];
}

/**
 * @brief Récupère une chaîne traduite dans une langue spécifique
 * @param key Clé de traduction
 * @param lang Langue souhaitée
 * @return Chaîne traduite dans la langue spécifiée
 */
static inline const char* tr_lang(translation_key_t key, language_t lang) {
  if (key >= KEY_COUNT || lang >= LANG_COUNT) {
    return "INVALID_KEY";
  }
  return translations[lang][key];
}

/**
 * @brief Obtient le nom de la langue
 * @param lang Langue
 * @return Nom de la langue
 */
static inline const char* get_language_name(language_t lang) {
  switch (lang) {
    case LANG_FR: return "Francais";
    case LANG_EN: return "English";
    case LANG_DE: return "Deutsch";
    default: return "Unknown";
  }
}

#endif  // LANG_H
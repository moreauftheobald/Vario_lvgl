#ifndef UI_TASK_H
#define UI_TASK_H

#include "global.h"
#include "ui.h"

// =============================================================================
// PARAMETRES TACHE UI
// =============================================================================

#define UI_UPDATE_RATE_MS 500    // Mise a jour 2 fois par seconde
#define UI_TASK_STACK 4096
#define UI_TASK_PRIORITY 2

static TaskHandle_t uiTaskHandle = NULL;

// =============================================================================
// TACHE UI
// =============================================================================

/**
 * @brief Tache FreeRTOS mise a jour UI
 */
void ui_task(void* parameter) {
#ifdef DEBUG_MODE
  ESP_LOGI("UI", "Demarrage tache");
#endif

  TickType_t last_wake_time = xTaskGetTickCount();

  while (1) {
    update_main_interface();
    
    vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(UI_UPDATE_RATE_MS));
  }
}

/**
 * @brief Cree la tache UI
 */
bool create_ui_task() {
  BaseType_t result = xTaskCreate(
    ui_task,
    "UI",
    UI_TASK_STACK,
    NULL,
    UI_TASK_PRIORITY,
    &uiTaskHandle);

  if (result != pdPASS) {
#ifdef DEBUG_MODE
    ESP_LOGE("UI", "Erreur creation tache");
#endif
    return false;
  }

#ifdef DEBUG_MODE
  ESP_LOGI("UI", "Tache creee");
#endif

  return true;
}

/**
 * @brief Arrete la tache UI
 */
void stop_ui_task() {
  if (uiTaskHandle != NULL) {
    vTaskDelete(uiTaskHandle);
    uiTaskHandle = NULL;
  }
}

#endif  // UI_TASK_H
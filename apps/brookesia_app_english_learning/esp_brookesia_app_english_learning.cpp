/*
 * SPDX-FileCopyrightText: 2023-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifdef ESP_UTILS_LOG_TAG
#   undef ESP_UTILS_LOG_TAG
#endif
#define ESP_UTILS_LOG_TAG "BS:App:English_Learning"
#include "esp_lib_utils.h"
#include "esp_brookesia_app_english_learning.hpp"
#include "ai_framework/agent/esp_brookesia_ai_agent.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"

#define ENGLISH_AGENT_INDEX 1

using namespace std;
using namespace esp_brookesia::gui;
using namespace esp_brookesia::speaker;
using namespace esp_brookesia::ai_framework;

LV_IMG_DECLARE(img_app_english_learning);

namespace esp_brookesia::speaker_apps {

English_Learning *English_Learning::requestInstance()
{
    if (_instance == nullptr) {
        ESP_UTILS_CHECK_EXCEPTION_RETURN(
            _instance = new English_Learning(), nullptr, "Failed to create instance"
        );
    }
    return _instance;
}

English_Learning::English_Learning():
    speaker::App("English Learning", &img_app_english_learning, true),
    _error_label(nullptr)
{
}

English_Learning::~English_Learning()
{
}

bool English_Learning::run(void)
{
    ESP_UTILS_LOGD("Run");

    // Check if chat service is available
    if (!Agent::requestInstance()->hasChatState(Agent::ChatState::ChatStateStarted)) {
        ESP_UTILS_LOGE("Chat service not started");
        showConnectionErrorUI("Chat service not available");
        return true; // Return true to keep app running with error UI
    }

    // First, switch to APP_AI mode to show the UI immediately
    // This provides better user experience by showing the interface first
    if (!getSystem()->manager.processDisplayScreenChange(
            ESP_BROOKESIA_SPEAKER_SCREEN_APP_AI, this)) {
        ESP_UTILS_LOGE("Failed to switch to APP_AI screen");
        showConnectionErrorUI("Failed to switch to APP_AI mode");
        return true;
    }
    ESP_UTILS_LOGI("Successfully switched to APP_AI mode, preparing English learning agent");

    // Wait a moment for UI to stabilize
    vTaskDelay(pdMS_TO_TICKS(200));

    // Save current agent index
    if (!Agent::requestInstance()->getCurrentRobotIndex(_original_agent_index)) {
        ESP_UTILS_LOGE("Failed to get current robot index");
        showConnectionErrorUI("Failed to get agent info");
        return true;
    }
    ESP_UTILS_LOGI("Saved original agent index: %d", _original_agent_index);

    // Switch to English learning agent
    if (!Agent::requestInstance()->setCurrentRobotIndex(ENGLISH_AGENT_INDEX)) {
        ESP_UTILS_LOGE("Failed to set English learning agent");
        showConnectionErrorUI("Failed to switch to English agent");
        return true;
    }
    ESP_UTILS_LOGI("Switched to English learning agent (index: %d)", ENGLISH_AGENT_INDEX);

    // Restart chat service to apply new agent
    if (Agent::requestInstance()->hasChatState(Agent::ChatState::ChatStateStarted)) {
        Agent::requestInstance()->sendChatEvent(Agent::ChatEvent::Stop);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    Agent::requestInstance()->sendChatEvent(Agent::ChatEvent::Start);

    // Wait for chat service to start with new agent
    int retry_count = 0;
    const int max_retries = 100; // Increased retry count for better reliability
    while (!Agent::requestInstance()->hasChatState(Agent::ChatState::ChatStateStarted) && retry_count < max_retries) {
        vTaskDelay(pdMS_TO_TICKS(200)); // Increased delay for more stable connection
        retry_count++;
        if (retry_count % 10 == 0) {
            ESP_UTILS_LOGD("Waiting for chat service to start... retry %d/%d", retry_count, max_retries);
        }
    }
    
    if (!Agent::requestInstance()->hasChatState(Agent::ChatState::ChatStateStarted)) {
        ESP_UTILS_LOGE("Failed to start chat service with English learning agent after %d retries", max_retries);
        showConnectionErrorUI("Chat service startup timeout");
        return true; // Return true to keep app running with error UI
    }
    ESP_UTILS_LOGI("Chat service started successfully with English learning agent");

    // Wait a moment for AI_Buddy to stabilize after resume
    vTaskDelay(pdMS_TO_TICKS(300));
    
    // Note: Long press exit is now handled by the system's default mechanism
    // The speaker manager will call back() method when long press is detected in APP_AI mode

    // Note: Do not send WakeUp event here as AI_Buddy's resume() method
    // will automatically handle the sleep/wake cycle when chat is started.
    // The AI will enter sleep state first, then can be woken up by user interaction.
    ESP_UTILS_LOGI("AI chat mode activated for English learning");

    return true;
}

void English_Learning::showConnectionErrorUI(const char *error_message)
{
    ESP_UTILS_LOGD("Showing connection error UI: %s", error_message);

    // Get the current screen (default screen created by core)
    lv_obj_t *screen = lv_scr_act();
    if (screen == nullptr) {
        ESP_UTILS_LOGE("Failed to get current screen");
        return;
    }

    // Clear existing error label if any
    if (_error_label != nullptr) {
        lv_obj_del(_error_label);
        _error_label = nullptr;
    }

    // Create main container
    lv_obj_t *container = lv_obj_create(screen);
    lv_obj_set_size(container, LV_PCT(90), LV_PCT(60));
    lv_obj_center(container);
    lv_obj_set_style_radius(container, 10, 0);
    lv_obj_set_style_bg_color(container, lv_color_hex(0x2C3E50), 0);
    lv_obj_set_style_border_width(container, 2, 0);
    lv_obj_set_style_border_color(container, lv_color_hex(0xE74C3C), 0);

    // Create title label
    lv_obj_t *title_label = lv_label_create(container);
    lv_label_set_text(title_label, "Connection Error");
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xE74C3C), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_20, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 20);

    // Create error message label
    _error_label = lv_label_create(container);
    lv_label_set_text(_error_label, error_message);
    lv_obj_set_style_text_color(_error_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(_error_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_align(_error_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(_error_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(_error_label, LV_PCT(80));
    lv_obj_align(_error_label, LV_ALIGN_CENTER, 0, 0);

    // Create retry instruction label
    lv_obj_t *retry_label = lv_label_create(container);
    lv_label_set_text(retry_label, "Please check network and try again");
    lv_obj_set_style_text_color(retry_label, lv_color_hex(0xBDC3C7), 0);
    lv_obj_set_style_text_font(retry_label, &lv_font_montserrat_12, 0);
    lv_obj_align(retry_label, LV_ALIGN_BOTTOM_MID, 0, -20);

    ESP_UTILS_LOGI("Connection error UI displayed: %s", error_message);
}

bool English_Learning::back(void)
{
    ESP_UTILS_LOGD("Back gesture detected, exiting English learning app");
    
    // Standard exit flow: notify core to close app, which will trigger processAppCloseExtra
    // This will automatically switch to MAIN screen (app selection interface)
    ESP_UTILS_CHECK_FALSE_RETURN(notifyCoreClosed(), false, "Notify core closed failed");
    
    return true;
}

bool English_Learning::close(void)
{
    ESP_UTILS_LOGD("Close - Application closing, handling cleanup and agent switching");

    // Step 1: Clean up UI state first
    if (_error_label != nullptr) {
        // Currently showing error UI, clean it up
        lv_obj_del(_error_label);
        _error_label = nullptr;
        ESP_UTILS_LOGD("Cleaned up error UI");
    }

    // Step 2: Handle Agent restoration before AI_Buddy operations
    // This ensures AI_Buddy operations work with correct chat state
    if (_original_agent_index >= 0) {
        // Currently in AI chat mode, restore original agent
        ESP_UTILS_LOGI("Restoring original agent (index: %d)", _original_agent_index);
        
        // Stop current chat service first
        if (Agent::requestInstance()->hasChatState(Agent::ChatState::ChatStateStarted)) {
            Agent::requestInstance()->sendChatEvent(Agent::ChatEvent::Stop);
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        
        // Restore original agent
        Agent::requestInstance()->setCurrentRobotIndex(_original_agent_index);
        
        // Restart chat service with original agent
        Agent::requestInstance()->sendChatEvent(Agent::ChatEvent::Start);
        
        // Brief wait for service to initialize
        vTaskDelay(pdMS_TO_TICKS(300));
        ESP_UTILS_LOGI("Agent restoration completed");
    }

    // Step 3: Reset AI_Buddy expression to neutral
    auto ai_buddy = esp_brookesia::speaker::AI_Buddy::requestInstance();
    if (ai_buddy != nullptr) {
        ESP_UTILS_LOGI("Resetting AI_Buddy expression to neutral");
        if (!ai_buddy->expression.setEmoji("neutral")) {
            ESP_UTILS_LOGW("Failed to reset AI_Buddy expression to neutral");
        }
    }

    ESP_UTILS_LOGI("Application cleanup and agent switching completed");

    return true;
}

bool English_Learning::pause(void)
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();
    ESP_UTILS_LOGI("Pausing English Learning app");

    // Get AI_Buddy instance
    auto ai_buddy = esp_brookesia::speaker::AI_Buddy::requestInstance();
    
    // Pause AI_Buddy to maintain stable state
    if (ai_buddy != nullptr && !ai_buddy->isPause()) {
        ESP_UTILS_LOGI("Pausing AI_Buddy during app pause");
        if (!ai_buddy->pause()) {
            ESP_UTILS_LOGW("Failed to pause AI_Buddy during app pause");
        }
    }

    // Set expression to neutral to avoid animation conflicts
    if (ai_buddy != nullptr) {
        ESP_UTILS_LOGI("Setting AI_Buddy expression to neutral during pause");
        if (!ai_buddy->expression.setEmoji("neutral")) {
            ESP_UTILS_LOGW("Failed to set AI_Buddy expression to neutral during pause");
        }
    }

    ESP_UTILS_LOGI("English Learning app paused successfully");
    return true;
}

bool English_Learning::resume(void)
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();
    ESP_UTILS_LOGI("Resuming English Learning app");

    // Get AI_Buddy instance
    auto ai_buddy = esp_brookesia::speaker::AI_Buddy::requestInstance();
    
    // Resume AI_Buddy if it was paused
    if (ai_buddy != nullptr && ai_buddy->isPause()) {
        ESP_UTILS_LOGI("Resuming AI_Buddy during app resume");
        if (!ai_buddy->resume()) {
            ESP_UTILS_LOGW("Failed to resume AI_Buddy during app resume");
        }
    }

    // Set appropriate expression for English learning context
    if (ai_buddy != nullptr) {
        ESP_UTILS_LOGI("Setting AI_Buddy expression to happy for English learning");
        if (!ai_buddy->expression.setEmoji("happy")) {
            ESP_UTILS_LOGW("Failed to set AI_Buddy expression to happy during resume");
        }
    }

    ESP_UTILS_LOGI("English Learning app resumed successfully");
    return true;
}

bool English_Learning::cleanResource(void)
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();
    ESP_UTILS_LOGI("Cleaning up English Learning app resources");

    // Clean up error UI if it exists
    if (_error_label != nullptr) {
        ESP_UTILS_LOGI("Cleaning up error label during resource cleanup");
        lv_obj_del(_error_label);
        _error_label = nullptr;
    }

    // Reset original agent index
    _original_agent_index = -1;

    ESP_UTILS_LOGI("English Learning app resources cleaned up successfully");
    return true;
}

// onAgentChatEventProcessEnd method removed - long press handling now done by system

} // namespace esp_brookesia::speaker_apps
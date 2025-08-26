/*
 * SPDX-FileCopyrightText: 2023-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "lvgl.h"
#include "esp_brookesia.hpp"
#include "ai_framework/agent/esp_brookesia_ai_agent.hpp"

namespace esp_brookesia::speaker_apps {

/**
 * @brief English Learning app for switching to English learning AI agent and providing
 *        an immersive English conversation experience.
 *
 */
class English_Learning: public esp_brookesia::speaker::App {
public:
    /**
     * @brief Destructor for the English Learning app
     *
     */
    ~English_Learning();

    /**
     * @brief Get the singleton instance of English_Learning
     *
     * @return Pointer to the singleton instance
     */
    static English_Learning *requestInstance();

protected:
    /**
     * @brief Private constructor to enforce singleton pattern
     *
     */
    English_Learning();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////// The following functions must be implemented by the user's app class. /////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief Called when the app starts running. This switches to the English learning agent
     *        and enters AI chat mode for English conversation practice.
     *
     * @return true if successful, otherwise false
     *
     */
    bool run(void) override;

    /**
     * @brief Called when the app receives a back event. This will exit the app and restore
     *        the original AI agent.
     *
     * @return true if successful, otherwise false
     *
     */
    bool back(void) override;

    /**
     * @brief Called when the app closes. This restores the original agent and returns to the main screen.
     *
     * @return true if successful, otherwise false
     *
     */
    bool close(void) override;

    /**
     * @brief Called when the app is paused. Handles AI_Buddy state and expression management.
     *
     * @return true if successful, otherwise false
     *
     */
    bool pause(void) override;

    /**
     * @brief Called when the app resumes. Restores AI_Buddy state and expression.
     *
     * @return true if successful, otherwise false
     *
     */
    bool resume(void) override;

    /**
     * @brief Called when the app starts to close. Performs extra resource cleanup.
     *
     * @return true if successful, otherwise false
     *
     */
    bool cleanResource(void) override;

private:
    int _original_agent_index = -1;
    lv_obj_t *_error_label = nullptr; // Error message label

    /**
     * @brief Show connection error UI with specified message
     *
     * @param error_message The error message to display
     */
    void showConnectionErrorUI(const char *error_message);

    // onAgentChatEventProcessEnd method removed - long press handling now done by system

    inline static English_Learning *_instance = nullptr; // Singleton instance
};

} // namespace esp_brookesia::speaker_apps
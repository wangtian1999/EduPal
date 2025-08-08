/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <string_view>
#include <array>
#include <vector>
#include <map>
#include "esp_brookesia.hpp"
#include "base.hpp"

namespace esp_brookesia::speaker_apps {

enum class SettingsUI_ScreenWlanSoftAPContainerIndex {
    QRCODE,
    MAX,
};

enum class SettingsUI_ScreenWlanSoftAPCellIndex {
    QRCODE_IMAGE,
    MAX,
};

struct SettingsUI_ScreenWlanSoftAPData {
    SettingsUI_WidgetCellContainerConf container_confs[(int)SettingsUI_ScreenWlanSoftAPContainerIndex::MAX];
    SettingsUI_WidgetCellConf cell_confs[(int)SettingsUI_ScreenWlanSoftAPCellIndex::MAX];
    struct {
        ESP_Brookesia_StyleSize_t main_size;
        ESP_Brookesia_StyleSize_t border_size;
        ESP_Brookesia_StyleColor_t dark_color;
        ESP_Brookesia_StyleColor_t light_color;
    } qrcode_image;
    struct {
        ESP_Brookesia_StyleSize_t size;
        ESP_Brookesia_StyleColor_t text_color;
        ESP_Brookesia_StyleFont_t text_font;
    } info_label;
};

using SettingsUI_ScreenWlanSoftAPCellContainerMap =
    SettingsUI_ScreenBaseCellContainerMap<SettingsUI_ScreenWlanSoftAPContainerIndex, SettingsUI_ScreenWlanSoftAPCellIndex>;

class SettingsUI_ScreenWlanSoftAP: public SettingsUI_ScreenBase {
public:
    SettingsUI_ScreenWlanSoftAP(speaker::App &ui_app, const SettingsUI_ScreenBaseData &base_data,
                                const SettingsUI_ScreenWlanSoftAPData &main_data);
    ~SettingsUI_ScreenWlanSoftAP();

    bool begin();
    bool del();
    bool processDataUpdate();

    lv_obj_t *getQRCodeImage() const
    {
        return _qrcode_image;
    }
    lv_obj_t *getInfoLabel() const
    {
        return _info_label;
    }

    static bool calibrateData(
        const ESP_Brookesia_StyleSize_t &parent_size, const ESP_Brookesia_CoreHome &home,
        SettingsUI_ScreenWlanSoftAPData &data
    );

    const SettingsUI_ScreenWlanSoftAPData &data;

private:
    bool processCellContainerMapInit();
    bool processCellContainerMapUpdate();

    SettingsUI_ScreenWlanSoftAPCellContainerMap _cell_container_map;

    lv_obj_t *_qrcode_image = nullptr;
    lv_obj_t *_info_label = nullptr;
};

} // namespace esp_brookesia::speaker

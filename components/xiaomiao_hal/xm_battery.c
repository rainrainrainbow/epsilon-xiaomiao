#include "xiaomiao_hal.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"

static const char* TAG = "xm_battery";
static adc_oneshot_unit_handle_t s_adc_handle = NULL;
static adc_cali_handle_t s_cali_handle = NULL;

void xm_battery_init(void) {
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_cfg, &s_adc_handle);

    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    adc_oneshot_config_channel(s_adc_handle, XM_BATTERY_ADC_CHANNEL, &chan_cfg);

    /* Use line fitting calibration (available in IDF v5.3) */
    adc_cali_line_fitting_config_t cali_cfg = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    adc_cali_create_scheme_line_fitting(&cali_cfg, &s_cali_handle);
    ESP_LOGI(TAG, "Battery ADC initialized (CH6/GPIO34)");
}

float xm_battery_voltage(void) {
    if (!s_adc_handle) return 0.0f;
    int raw;
    adc_oneshot_read(s_adc_handle, XM_BATTERY_ADC_CHANNEL, &raw);
    int voltage_mv = 0;
    if (s_cali_handle) {
        adc_cali_raw_to_voltage(s_cali_handle, raw, &voltage_mv);
    }
    /* Voltage divider on XiaoMiao: assume 2:1 divider -> multiply by 2 */
    return (float)voltage_mv * 2.0f / 1000.0f;
}

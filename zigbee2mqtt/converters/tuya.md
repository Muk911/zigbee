# Конвертеры для устройств Tuya #

https://www.zigbee2mqtt.io/advanced/support-new-devices/02_support_new_tuya_devices.html

https://developer.tuya.com/en/docs/iot/tuya-zigbee-module-uart-communication-protocol?id=K9ear5khsqoty

https://github.com/DzurisHome/Tuya-Data-Points

https://pypi.org/project/tinytuya/

https://github.com/dulfer/localtuya-device-datapoints?ysclid=lssp7i48vc104440240

```
const tuyaTz = {

    datapoints: {
        key: [
            'temperature_unit', 'temperature_calibration', 'humidity_calibration', 'alarm_switch', 'tamper_alarm_switch',
            'state', 'brightness', 'min_brightness', 'max_brightness', 'power_on_behavior', 'position', 'alarm_melody', 'alarm_mode',
            'countdown', 'light_type', 'silence', 'self_test', 'child_lock', 'open_window', 'open_window_temperature', 'frost_protection',
            'system_mode', 'heating_stop', 'current_heating_setpoint', 'local_temperature_calibration', 'preset', 'boost_timeset_countdown',
            'holiday_start_stop', 'holiday_temperature', 'comfort_temperature', 'eco_temperature', 'working_day',
            'week_schedule_programming', 'online', 'holiday_mode_date', 'schedule', 'schedule_monday', 'schedule_tuesday',
            'schedule_wednesday', 'schedule_thursday', 'schedule_friday', 'schedule_saturday', 'schedule_sunday', 'clear_fault',
            'scale_protection', 'error', 'radar_scene', 'radar_sensitivity', 'tumble_alarm_time', 'tumble_switch', 'fall_sensitivity',
            'min_temperature', 'max_temperature', 'window_detection', 'boost_heating', 'alarm_ringtone', 'alarm_time', 'fan_speed',
            'reverse_direction', 'border', 'click_control', 'motor_direction', 'opening_mode', 'factory_reset', 'set_upper_limit', 'set_bottom_limit',
            'motor_speed', 'timer', 'reset_frost_lock', 'schedule_periodic', 'schedule_weekday', 'schedule_holiday', 'backlight_mode', 'calibration',
            'motor_steering', 'mode', 'lower', 'upper', 'delay', 'reverse', 'touch', 'program', 'light_mode', 'switch_mode',
            ...[1, 2, 3, 4, 5, 6].map((no) => `schedule_slot_${no}`), 'minimum_range', 'maximum_range', 'detection_delay', 'fading_time',
            'radar_sensitivity', 'entry_sensitivity', 'illuminance_threshold', 'detection_range', 'shield_range', 'entry_distance_indentation',
            'entry_filter_time', 'departure_delay', 'block_time', 'status_indication', 'breaker_mode', 'breaker_status',
            'alarm', 'alarm_time', 'alarm_volume', 'type', 'volume', 'ringtone', 'duration', 'medium_motion_detection_distance',
            'large_motion_detection_distance', 'large_motion_detection_sensitivity', 'small_motion_detection_distance',
            'small_motion_detection_sensitivity', 'static_detection_distance', 'static_detection_sensitivity', 'keep_time', 'indicator',
            'motion_sensitivity', 'detection_distance_max', 'detection_distance_min', 'presence_sensitivity', 'sensitivity', 'illuminance_interval',
            'medium_motion_detection_sensitivity', 'small_detection_distance', 'small_detection_sensitivity', 'fan_mode', 'deadzone_temperature',
            'eco_mode', 'max_temperature_limit', 'min_temperature_limit', 'manual_mode',
            'medium_motion_detection_sensitivity', 'small_detection_distance', 'small_detection_sensitivity', 'switch_type',
            'ph_max', 'ph_min', 'ec_max', 'ec_min', 'orp_max', 'orp_min', 'free_chlorine_max', 'free_chlorine_min', 'target_distance',
            'illuminance_treshold_max', 'illuminance_treshold_min', 'presence_illuminance_switch', 'light_switch', 'light_linkage',
            'indicator_light', 'find_switch', 'detection_method', 'sensor', 'hysteresis', 'max_temperature_protection', 'display_brightness',
            'screen_orientation', 'regulator_period', 'regulator_set_point', 'upper_stroke_limit', 'middle_stroke_limit', 'lower_stroke_limit',
            'buzzer_feedback', 'rf_pairing', 'max_temperature_alarm', 'min_temperature_alarm', 'max_humidity_alarm', 'min_humidity_alarm',
            'temperature_periodic_report', 'humidity_periodic_report', 'temperature_sensitivity', 'humidity_sensitivity', 'temperature_alarm',
            'humidity_alarm', 'move_sensitivity', 'radar_range', 'presence_timeout',
        ],
        convertSet: async (entity, key, value, meta) => {

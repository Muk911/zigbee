## Устройства на базе ESP32-C6/H2 ##
Модули ESP32-C6 и ESP32-H2 от китайской компании Expressif унаследовали все прелести традиционных ESP32-WROOM, но дополнительно снабжены Zigbee, что делает их удобной базой для реализации единичных Zigbee-устройств.
Но есть и некоторые минусы:
- Сырое ПО. В настоящее время компания Expressif продолжает дорабатывать библиотеки и IDE для ESP32-C6/H2, соответственно часть стандарта Zigbee Cluster Library может оказаться нереализованной.
- Высокое энергопотребление модулей и недоработанные режимы сна. Реализация устройств с батарейным питанием может оказаться вызовом для разработчика.
### ПО для программирования ###
Для разработки прошивок под модули ESP32-C6/H2 компания Expressif предоставляет open-source продукт [Expressif IoT Development Framework (IDF)](https://github.com/espressif/esp-idf).
Входящий в него Expressif IDE представляет собой надстройку над Eclipse. Доступно также расширение Expressif IDF для VS Code+PlatformIO.

Шаги по установке и настройке Expressif IDF описаны в статьях:
- https://www.waveshare.com/wiki/ESP32-C6-DEV-KIT-N8
- https://www.waveshare.com/wiki/ESP32-H2-DEV-KIT-N4
- https://wiki.seeedstudio.com/xiao_esp32c6_zigbee/
- https://wiki.seeedstudio.com/xiao_esp32c6_zigbee_arduino/
- https://community.home-assistant.io/t/help-with-esp32-h2-zigbee/766216/9

Для начального старта можно использовать также известный Arduino IDE и библиотеку Arduino-ESP32. Ниже описаны шаги по настройке Arduino IDE для разработки под ESP32-C6/H2.

Еще одной альтернативой является использование Expressif IDF и подключение библиотек Arduino как компонентов IDF.

### Установка Arduino IDE и расширения для ESP32-C6/H2 ###
1. Скачайте и установите актуальную версию Arduino IDE. На момент написания этого текста последней версией была версия 2.2.1.
2. В диалоге настроек **Preferences** в разделе **Additional boards manager URLs** укажите ссылку для Arduino-ESP32 (ссылка для DEV-версии: https://espressif.github.io/arduino-esp32/package_esp32_dev_index.json).
3. Вызовите менеджер плат Boards Manager. В поиске наберите "esp32" и найдите библиотеку плат "esp32 by Expressif Systems". На момент написания этого текста последней версией была версия 3.0.0-alpha3. Установите библиотеку плат.
4. В меню Tools выберите подходящий модуль, например Board > esp32 > **ESP32C6 Dev Module**.
5. В меню Tools выберите Partition Scheme = "Zigbee 4MB with spiffs".
6. В меню Tools выберите Zigbee Mode = "Zigbee ED (end device)". Предполагается, что Вы хотите разработать конечное устройство Zigbee. На момент написания этого текста разработка прошивки для роутера в Arduino-ESP32 была недоступна.
7. Для возможности использования отладочной информации (например, функций ESP_LOGI() или log_i()) выберите в меню Tools значение Core Debug Level в зависимости от желаемого уровня отладки.

### Подключение модуля ###
1. Подключите модуль ESP32-C6/H2 к ПК через UART. На отладочных модулях ESP32-C6-DevKit имеется встроенный преобразователь USB-UART, выведенный на USB Type-C разъем с надписью "UART". Для подключения "голых" модулей необходим внешний преобразователь USB-UART.
2. Выберите порт модуля в Arduino IDE. При необходимости перезапустите Arduino IDE для обновления списка доступных портов.

### Загрузка прошивки ###
1. Загрузите в IDE текстовый скетч Blink и выполните загрузку прошивки (Upload) в модуль. Отладочные модули ESP32-C6-DevKit обеспечивают автоматическое притягивание к "0" вывода GPIO9 для перевода чипа в режим загрузки прошивки. Перемычка отключения периферии должна быть установлена. Если у Вас "голый" модуль или по каким-то причинам автоматический перевод чипа в режим загрузки прошивки не выполняется, необходимо притянуть к "0" вывод GPIO9 (нажать и удерживать кнопку BOOT при ее наличии), сбросить чип кратковременным притягиванием вывода EN к "0" (кратковременно нажать кнопку RESET при ее наличии). После этого модуль может быть отсоединен от "0".
2. После успешной загрузки скетча Blink на модуле будет мигать встроенный светодиод.
   
### Разработка Zigbee-устройств ###
1. Далее можно перейти к реализации первого устройства Zigbee. Для успешной разработки необходимо изучить принципы использования FreeRTOS и основы стандарта Zigbee Cluster Library.
2. В репозитории приведен пример скетча zed_sample.ino для конечного устройства Zigbee, реализующего одну конечную точку (endpoint) и два стандартных кластера - OnOff и Temperature Measurement. Реализация кластера OnOff имитирует работу умного реле, а реализация кластера Temperature Measurement имитирует работу сенсора температуры. При переключении состояния "реле" происходит включение/выключение встроенного светодиода ESP32-C6. Изменение  температуры имитируется с помощью генератора случайных чисел.

### Тестовый стенд для устройств Zigbee ###
1. Необходимо заранее подготовить тестовый стенд для устройств Zigbee. Например, это может быть Home Assistant с дополнением Zigbee Home Automation (ZHA) или Zigbee2MQTT. Рекомендуется проверить работу тестового стенда на любом заведомо исправном устройстве.
2. В случае использования в разрабатываемом устройстве стандартных кластеров и атрибутов Zigbee Cluster Library, устройство должно подключаться к тестовому стенду без необходимости написания каких-либо конвертеров.
3. Следует напомнить, что для получения данных с сенсоров необходимо настроить режим репортинга. Некоторые системы Умного дома обеспечивают настройку репортинга по умолчанию. Если данные сенсоров все такие не обновляются, необходимо настроить репортинг самостоятельно.

### Примеры кода ###
- https://github.com/espressif/esp-idf/tree/master/examples/zigbee
- https://github.com/espressif/esp-zigbee-sdk/tree/main/examples
- https://github.com/espressif/arduino-esp32/tree/master/libraries/ESP32/examples/Zigbee
- https://github.com/espressif/arduino-esp32/tree/master/libraries/Zigbee
- https://github.com/u236/esp32c6-light-example
- https://github.com/u236/esp-zb-vindriktning
- https://github.com/lmahmutov/esp32_c6_co2_sensor
- https://github.com/kylemwagner83/esp32c6-bme280-zigbee
- https://github.com/nomis/candle-dribbler
- https://github.com/Live-Control-Project/MEEF
- https://github.com/xmow49/ESP32H2-Zigbee-Demo
- https://github.com/xmow49/ESP32H2-SmokeDetector
- https://github.com/barsik21/esp32h2-thermo
- https://github.com/username-AL/ESP32H2-Zigbee
- https://github.com/username-AL/ESP32H2-SmokeDetector
- https://github.com/DmytroML/ESP36C6_Zigbee_PZEM-004t
- https://github.com/RJSDevel/IKEA_VINDRIKTNING_ESP32_ZIGBEE
- https://github.com/RJSDevel/HA_switch_2_gang_wt0132c6-s5
- https://github.com/ginkage/CeilingCW-C6
- https://gitlab.com/kasraghu/p1-esp32c6
- https://github.com/rpavlyuk/ESPPressureSensor
- https://github.com/GainSec/M5NanoC6-Zigbee-Sniffer
- https://github.com/allexoK/Esp32-C6-Bug-Arduino-Examples
- https://github.com/luar123/esphome_zb_sensor
- https://github.com/wokwi/esp32c6-i2c-lp
- https://github.com/mundevx/Light_Dim_Color_Fan
- https://github.com/vedatbotuk/selforganized_802.15.4_network_with_esp32
- https://github.com/mozolin/esp32c6_zigbee
- https://github.com/m5stack/M5NanoC6/tree/main/examples (примеры для C6 без использования Zigbee)
- https://github.com/nlimper/Mini-AP-v3
- https://github.com/nlimper/Mini-AP-v4
- https://github.com/prairiesnpr/esp_zha_freezer_mon
- https://github.com/diogocardoso28/MotorradCan2Zigbee
- https://github.com/DmytroML/ESP36H2_Zigbee_PZEM-003
- https://github.com/DmytroML/ESP36C6_Zigbee_PZEM-004t
- https://github.com/jorritohm/ESP32-C6_ZigBee_Sensor-and-Display
- https://github.com/scipioni/beesensor
- https://github.com/prairiesnpr/esp_zha_test_bench
- https://github.com/tlanglois1111/zigbee_aht20
- https://github.com/markot99/airbee
- https://github.com/GavriKos/ZigBee-irrigation-automation
- https://github.com/ArtemProc/ESP32-c6-zb-esp-idf
- https://wiki.seeedstudio.com/xiao_esp32c6_zigbee_arduino/
- https://github.com/volkerp/ESP32C6_Zigbee_Example
- https://github.com/allexoK/Esp32-C6-Bug-Arduino-Examples

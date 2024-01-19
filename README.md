# Вступление #
В этом репозитории я решил собрать результаты своих изысканий в области Zigbee-устройств для Умного дома.
## Устройства на базе ESP32-C6/H2 ##
Модули ESP32-C6 и ESP32-H2 от китайской компании Expressif унаследовали все прелести традиционных ESP32-WROOM, но дополнительно снабжены Zigbee, что делает их удобной базой для реализации единичных Zigbee-устройств.
Но есть и некоторые минусы:
- Сырое ПО. В настоящее время компания Expressif продолжает дорабатывать библиотеки и IDE для ESP32-C6/H2, соответственно часть стандарта Zigbee Cluster Library может оказаться нереализованной.
- Высокое энергопотребление модулей и недоработанные режимы сна. Реализация устройств с батарейным питанием может оказаться вызовом для разработчика.
### ПО для программирования ###
Для разработки прошивок под модули ESP32-C6/H2 компания Expressif предоставляет open-source продукт [Expressif IoT Development Framework (IDF)](https://github.com/espressif/esp-idf).
Входящий в него Expressif IDE представляет собой надстройку над Eclipse. Доступно также расширение Expressif IDF для VS Code+PlatformIO.

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
1. Загрузите в IDE текстовый скетч Blink и выполните загрузку прошивки (Upload) в модуль. Отладочные модули ESP32-C6-DevKit обеспечивают автоматическое притягивание к "0" вывода GPIO9 для перевода чипа в режим загрузки прошивки. Перемычка отключения периферии должна быть установлена. Если у Вас "голый" модуль или по каким-то причинам автоматический перевод чипа в режим загрузки прошивки не выполняется, необходимо притянуть к "0" вывод GPIO9 (нажать и удерживать кнопку BOOT при ее наличии), сбросить чип кратковременным притягиванием вывода EN к "0" (кратковременно нажать кнопку RESET при ее наличии).
2. После успешной загрузки скетча Blink на модуле будет мигать встроенный светодиод.
### Разработка Zigbee-устройств ###
1. Далее можно перейти к реализации первого устройства Zigbee. Для успешной разработки необходимо изучить принципы использования FreeRTOS и основы стандарта Zigbee Cluster Library.
2. В репозитории приведен пример скетча zed_sample.ino для конечного устройства Zigbee, реализующего одну конечную точку (endpoint) и два стандартных кластера - OnOff и Temperature Measurement. Реализация кластера OnOff имитирует работу умного реле, а реализация кластера Temperature Measurement имитирует работу сенсора температуры. При переключении состояния "реле" происходит включение/выключение встроенного светодиода. Изменение  температуры имитируется с помощью генератора случайных чисел.
### Тестовый стенд для устройств Zigbee ###
1. Необходимо заранее подготовить тестовый стенд для устройств Zigbee. Например, это может быть Home Assistant с дополнением Zigbee Home Automation (ZHA) или Zigbee2MQTT. Рекомендуется проверить работу тестового стенда на любом заведомо исправном устройстве.
2. В случае использования в разрабатываемом устройстве стандартных кластеров и атрибутов Zigbee Cluster Library, устройство должно подключаться к тестовому стенду без необходимости написания каких-либо конвертеров.
3. Следует напомнить, что для получения данных с сенсоров необходимо настроить режим репортинга. Некоторые системы Умного дома обеспечивают настройку репортинга по умолчанию. Если данные сенсоров все такие не обновляются, необходимо настроить репортинг самостоятельно.

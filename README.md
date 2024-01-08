# Вступление #
В этом репозитории я решил собрать результаты своих изысканий в области Zigbee-устройств для Умного дома.
## Устройства на базе ESP32-C6/H2 ##
Модули ESP32-C6 и ESP32-H2 от китайской компании Expressif унаследовали все прелести традиционных ESP32-WROOM, но дополнительно снабжены Zigbee, что делает их удобной базой для реализации единичных Zigbee-устройств.
Но есть и некоторые минусы:
- Сырое ПО. В настоящее время компания Expressif продолжает дорабатывать библиотеки и IDE для ESP32-C6/H2, соответственно часть стандарта Zigbee Cluster Library может оказаться нереализованной.
- Высокое энергопотребление модулей и недоработанные режимы сна. Реализация устройств с батарейным питанием может оказаться вызовом для разработчика.
### ПО для программирования ###
Для разработки прошивок под модули ESP32-C6/H2 компания Expressif предоставляет open-source продукт [Expressif IoT Development Framework (IDF)](https://github.com/espressif/esp-idf).
Входящий в него Expressif IDE представляет собой надстройку над Eclipse. Доступны также расширения Expressif IDF для VS Code и PlatformIO.
Для начального старта можно использовать также известный Arduino IDE через библиотеку Arduino-ESP32. Ниже описаны шаги по настройке Arduino IDE для разработки под ESP32-C6/H2.
### Установка Arduino IDE и расширения ESP32-C6/H2 ###
1. Скачайте и установите актуальную версию Arduino IDE. На момент написания этого текста последней версией была версия 2.2.1.
2. В диалоге настроек **Preferences** в разделе **Additional boards manager URLs** укажите ссылку для Arduino-ESP32 (ссылка для DEV-версии: https://espressif.github.io/arduino-esp32/package_esp32_dev_index.json).
3. Вызовите менеджер плат Boards Manager. В поиске наберите "esp32" и найдите библиотеку плат "esp32 by Expressif Systems". На момент написания этого текста последней версией была версия 3.0.0-alpha3. Установите библиотеку плат.
4. В меню Tools выберите подходящий модуль, например Board > esp32 > **ESP32C6 Dev Module**.
5. В меню Tools выберите Partition Scheme = "Zigbee 4MB with spiffs".
6. В меню Tools выберите Zigbee Mode = "Zigbee ED (end device)". Предполагается, что Вы хотите разработать конечное устройство Zigbee. На момент написания этого текста разработка прошивки для роутера была недоступна.
7. Для возможности использования отладочной информации (например, функций ESP_LOGI() или log_i()) в меню Tools выберите значение Core Debug Level в зависимости от желаемого уровня отладки.
8.  

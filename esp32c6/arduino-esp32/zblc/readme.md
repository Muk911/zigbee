# Zigbee Low Code #
Под этим рабочим названием я решил выложить свои классы, предназначенные для разработки устройств Zigbee с минимальным количеством кодирования.

## Установка ##
Распакуйте содержимое архива zblc.zip в папку \Documents\Arduino\libraries, куда Arduino IDE записывает все устанавливаемые библиотеки.
Требуемая структура папок:
- Arduino
  - libraries
    - zblc
      - examples
      - src

После очередного запуска Arduino IDE в меню File>Examples должен появиться раздел "zblc". Откройте интересующий пример и настройте для него параметры в меню Tools.

## Зависимости ##
Zigbee Low Code использует библиотеку OneButton для реализации сервисной кнопки (см. файлы zb_button.*).

## Компиляция ##
Если при компиляции возникает ошибка 
```
\esp32c6/include/espressif__esp-zigbee-lib/include/zcl/esp_zigbee_zcl_core.h:259:54:
error: 'esp_zb_zcl_command_send_status_callback_t' was not declared in this scope
```
закомментируйте указанную в ошибке строку файла esp_zigbee_zcl_core.h

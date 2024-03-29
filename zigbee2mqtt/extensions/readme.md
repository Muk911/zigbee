# Расширения Z2M #

Расширения Z2M представляют собой файлы с кодом на языке JavaScript. Файл расширения содержит описание класса, конструктор которого должен соответствовать определенным соглашениям. Существенная часть функциональности Z2M реализована в [форме расширений](https://github.com/Koenkk/zigbee2mqtt/tree/master/lib/extension).

Z2M поддерживает подключение пользовательских ("внешних") расширений, которые добавляются в Z2M на странице "Расширения".

## Расширение для перехвата сообщений ##

Расширение [debug-device.js](debug-device.js) позволяет перехватывать сообщения от выбранного Zigbee-устройства. Это не прослушивание эфира, как при использовании сниффера, это просто хук Z2M, который отслеживает все сообщения, поступающие с координатора от выбранного устройства, и отображает атрибуты сообщений во всплывающих окнах. Отслеживаемое устройство должно быть зарегистрировано в Z2M.

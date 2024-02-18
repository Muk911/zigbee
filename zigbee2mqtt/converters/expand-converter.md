# Расширение существующего конвертера Z2M #
Во многих случаях добавление поддержки нового устройства в Z2M заключается в расширении списка моделей в "отпечатке пальцев" (fingerprint) определения устройства в файле конвертера. 
В некоторых случаях конвертер может быть реализован заимствованием частей кода из библиотеки [zigbee-herdsman-converters](https://github.com/Koenkk/zigbee-herdsman-converters).
И совсем редко конвертер разрабатывается полностью с нуля.

В данной статье описаны подходы, позволяющие разработать внешний конвертер Z2M на основе существующего конвертера, путем замены отдельных частей кода конвертера.

## Добавление расширение Fingerprint ##
Многие китайские производители выпускают обновления существующих моделей устройств с другими значениями производителя и модели устройства. Если функционально устройство не менялось, достаточно подготовить файл внешнего конвертера, расширяющий список моделей для существующего определения устройства.

Первым шагом необходимо получить ссылку на определение (definition) существующего устройства, которое подходит для нашего нового устройства.
Это можно сделать несколькими способами, например так:
```
const zhc = require('zigbee-herdsman-converters');
const baseDefinition = zhc.definitions.find((d) => d.model === 'BHT-002-GCLZB');
```
или так:
```
const zhc = require('zigbee-herdsman-converters');
const baseDefinition = zhc.findByModel('BHT-002-GCLZB');
```
Теперь необходимо создать новое определение устройства на основе базового определения. Следующее присвоение создает копию базового определения. 
```
const definition = {...baseDefinition};
```
Благодаря оператору расширения (...) копируется не ссылка, а вся структура определения, что позволяет производить модификацию любых элементов нового определения, не затрагивая исходное определение.

Далее заменяем значения атрибутов определения на нужные нам:
```
definition.fingerprint = [{modelID: 'TS0601', manufacturerName: '_TZE200_o9d1hdma'}];
definition.model = 'AE-668D-Zigbee';
definition.vendor = 'Acmelec';
definition.description = 'Acmelec AE-668D Zigbee Thermostat';
```
Это же можно написать более лаконично:
```
const definition = {
  ...baseDefinition,
  fingerprint: [{modelID: 'TS0601', manufacturerName: '_TZE200_o9d1hdma'}],
  model: 'AE-668D-Zigbee',
  vendor: 'Acmelec',
  description: 'Acmelec AE-668D Zigbee Thermostat',
};
```
Здесь сразу выполняется копирование определения и заменяются значения атрибутов.

Последней строкой конвертера необходимо экспортировать наше новое определение:
```
module.exports = definition;
```
## Изменение логики преобразования ##
Если в новой модели устройства произошли более существенные изменения, то в новом определении устройства необходимо реализовать добавление (или удаление) элементов или замену существующих элементов на их новую версию.
Рассмотрим кейс, когда новая версия термостата экосистемы Tuya передает значение атрибута local_temperature с другим множителем.

За базовое определение устройства выбрана популярная модель термостата Moes BHT-002-GCLZB.
Преобразование данных, передаваемых из устройства в MQTT, выполняется функциями-конвертерами, массив которых содержится в атрибуте fromZigbee определения устройства.
Файл [конвертеров Moes](https://github.com/Koenkk/zigbee-herdsman-converters/blob/master/src/devices/moes.ts) для модели BHT-002-GCLZB содержит всего одну такую функцию:
```
fromZigbee: [legacy.fz.moes_thermostat],
```
Сама функция-конвертер moes_thermostat находится в файле [legacy.ts](https://github.com/Koenkk/zigbee-herdsman-converters/blob/master/src/lib/legacy.ts).

Если быть точнее, moes_thermostat - это структура, содержащая кластер, тип команды ZCL, и указатель convert на функцию преобразования.
Внутри функции находится оператор switch(dp){}, выполняющий ветвление в зависимости от значения dataPoint. Особенности реализации устройств Tuya будут рассмотрены в другой статье, здесь хочется показать общий подход к изменению логики преобразования.

Предварительно сохраняем указатель на существующую реализацию функции преобразования в вспомогательном атрибуте определения устройства.
```
definition.convertBase = definition.fromZigbee[0].convert;
```
Затем заменяем существующую реализацию функции преобразования новой, в которой происходит преобразование только для локальной температуры, а для остальных dataPoints вызывается базовая функция.
```
  definition.fromZigbee[0] = {
      ...definition.fromZigbee[0],
      convert: (model, msg, publish, options, meta) => {
          const dpValue = msg.data.dpValues[0]; 
          if(dpValue.dp == legacy.dataPoints.moesLocalTemp) {
              const value = legacy.getDataValue(dpValue);
              let temperature = value & 1<<15 ? value - (1<<16) + 1 : value;
              temperature = parseFloat(temperature.toFixed(1));
              if (temperature < 100) {
                  return {local_temperature: parseFloat(temperature.toFixed(1))};
              }
          }
          else {
              meta.device.definition.convertBase(model, msg, publish, options, meta);
          }
      }     
  }
```

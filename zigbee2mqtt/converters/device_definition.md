### Описание устройства во внешнем конверторе Z2M ###
Описание устройства представляет собой структуру JavaScript, содержащую предопределенный набор значений.
| Переменная   | Описание | Использование |
|--------------|----------|---------------|
| vendor       | Наименование производителя | Отображается на карточке устройства    |
| model        | Код модели  | Отображается на карточке устройства    |
| zigbeeModel  | ??? |
| description  | Описание модели | Отображается на карточке устройства    |
| fingerprint  | Массив "отпечатков пальцев" устройства | См. ниже |
| whiteLabel   | Бренды и марки, под которыми выпускаются устройства | ??? |
| fromZigbee   | Массив конвертеров для преобразования данных, пришедших от устройства | См. далее |
| toZigbee     | Массив конвертеров для преобразования данных, отправляемых на устройство | См. далее |
| exposes      | Массив сущностей, используемых для представления устройства в Z2M |
| extend       | 
| meta         | Структура с метаданными устройства | См. далее |
| endpoint     | Функция, возвращающая список конечных точек устройства | Используется для отображения имен конечных точек в их реальные номера | 
| configure    | Функция конфигурирования устройства.
| ota          | 

```
{
zigbeeModel: ['EGLO_ZM_TW'],
fingerprint: [
    {
        type: 'Router', manufacturerName: 'AwoX', modelID: 'TLSR82xx', endpoints: [
            {ID: 1, profileID: 260, deviceID: 268, inputClusters: [0, 3, 4, 5, 6, 8, 768, 4096], outputClusters: [6, 25]},
            {ID: 3, profileID: 49152, deviceID: 268, inputClusters: [65360, 65361], outputClusters: [65360, 65361]},
        ],
    },
],
model: '33955',
vendor: 'AwoX',
description: 'LED light with color temperature',
extend: [light({colorTemp: {range: [153, 370]}})],
whiteLabel: [{vendor: 'EGLO', model: '900316'}, {vendor: 'EGLO', model: '900317'}, {vendor: 'EGLO', model: '900053'}],
}
```

#### "Отпечатки пальцев" устройства ####
Во многих случаях однозначный выбор определения устройства по атрибутам manufacturerName и modelIdentifier базового кластера устройства невозможен. Например, под одним номеров модели могут выпускаться устройства с различным количеством конечных точек. Или с отличающимся набором кластеров. В таких случаях для правильного выбора определения устройства применяются "отпечатки пальцев". Это массив специальных структур, содержащих значения и комбинации значений тех атрибутов, которые позволяют различить похожие устройства.
Примеры использования "отпечатков пальцев":
```
fingerprint: [{modelID: 'ON/OFF(2CH)', softwareBuildID: '2.9.2_r54'}],
fingerprint: [{modelID: 'GreenPower_254', ieeeAddr: /^0x00000000005.....$/}],
```
Более сложный пример:
```
fingerprint: [
    {
        type: 'Router', manufacturerName: 'AwoX', modelID: 'TLSR82xx', endpoints: [
            {ID: 1, profileID: 260, deviceID: 268, inputClusters: [0, 3, 4, 5, 6, 8, 768, 4096, 64599], outputClusters: [6]},
            {ID: 3, profileID: 4751, deviceID: 268, inputClusters: [65360, 65361], outputClusters: [65360, 65361]},
        ],
    },
    {
        type: 'Router', manufacturerName: 'AwoX', modelID: 'TLSR82xx', endpoints: [
            {ID: 1, profileID: 260, deviceID: 268, inputClusters: [0, 3, 4, 5, 6, 8, 768, 4096, 64599], outputClusters: [6]},
            {ID: 242, profileID: 41440, deviceID: 97, inputClusters: [], outputClusters: [33]},
            {ID: 3, profileID: 4751, deviceID: 268, inputClusters: [65360, 65361], outputClusters: [65360, 65361]},
        ],
    },
    {
        type: 'Router', manufacturerName: 'AwoX', modelID: 'TLSR82xx', endpoints: [
            {ID: 1, profileID: 260, deviceID: 268, inputClusters: [0, 3, 4, 5, 6, 8, 768, 4096, 64599, 10], outputClusters: [6]},
            {ID: 242, profileID: 41440, deviceID: 97, inputClusters: [], outputClusters: [33]},
            {ID: 3, profileID: 4751, deviceID: 268, inputClusters: [65360, 65361], outputClusters: [65360, 65361]},
        ],
    },
    {
        type: 'Router', manufacturerName: 'AwoX', modelID: 'TLSR82xx', endpoints: [
            {ID: 1, profileID: 260, deviceID: 268, inputClusters: [0, 3, 4, 5, 6, 8, 768, 4096, 64599, 10], outputClusters: [6]},
            {ID: 242, profileID: 41440, deviceID: 97, inputClusters: [], outputClusters: [33]},
            {ID: 3, profileID: 4751, deviceID: 268, inputClusters: [65360, 65361, 4], outputClusters: [65360, 65361]},
        ],
    },
],
```
В коде конвертеров могут встречаться "отпечатки пальцев", заданные через функции. Это сделано для лаконичности записи. Например в примере ниже идентификатор модели указан только один раз:
```
fingerprint: tuya.fingerprint('TS130F', ['_TZ3000_j1xl73iw', '_TZ3000_kmsbwdol', '_TZ3000_esynmmox', '_TZ3000_l6iqph4f', '_TZ3000_xdo0hj1k']),
```
Соответствующая функция из модуля tuya.ts обогащает элементы массива по втором параметре значением первого параметра. В результате получается массив:
```
fingerprint: [{modelID: 'TS130F', manufacturerName: '_TZ3000_j1xl73iw'}, {modelID: 'TS130F', manufacturerName: '_TZ3000_kmsbwdol'}, {modelID: 'TS130F', manufacturerName: '_TZ3000_esynmmox'}, {modelID: 'TS130F', manufacturerName: '_TZ3000_l6iqph4f'}, {modelID: 'TS130F', manufacturerName: '_TZ3000_xdo0hj1k'}],
```
#### White Label ####
Пример использования whiteLabel:
```
whiteLabel: [
    {
        model: 'K4027C/L4027C/N4027C/NT4027C', vendor: 'BTicino', description: 'Shutter SW with level control',
        fingerprint: [{hardwareVersion: 9}, {hardwareVersion: 13}],
    },
],
```

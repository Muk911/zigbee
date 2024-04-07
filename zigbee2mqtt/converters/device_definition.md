### Определение устройства во внешнем конверторе Z2M ###
Определение устройства представляет собой структуру JavaScript, содержащую предопределенный набор переменных:
| Переменная   | Описание | Использование |
|--------------|----------|---------------|
| vendor       | Наименование производителя | Отображается на карточке устройства    |
| model        | Код модели | Отображается на карточке устройства    |
| zigbeeModel  | Идентификатор модели, которым устройство представляется при интервью | См. далее |
| description  | Описание модели | Отображается на карточке устройства    |
| fingerprint  | Массив "отпечатков пальцев" устройства | См. ниже |
| whiteLabel   | Бренды и марки, под которыми выпускаются устройства | См. далее |
| fromZigbee   | Массив объектов-конвертеров для преобразования данных, пришедших от устройства | См. далее |
| toZigbee     | Массив объектов-конвертеров для преобразования данных, отправляемых на устройство | См. далее |
| onEvent      | Функция обработки низкоуровневых событий Z2M | См. далее |
| exposes      | Массив объектов-сущностей, используемых для представления устройства в Z2M | См. далее |
| extend       | Массив объектов-расширений, отвечающих за комплексное определение устройства | См. далее |
| meta         | Структура с метаданными устройства | См. далее |
| endpoint     | Функция, возвращающая список конечных точек устройства | См. далее | 
| configure    | Функция конфигурирования устройства. | См. далее |
| ota          | 
| icon         | Миниатюрное изображение устройства в JPG, закодированное Base64 | Отображается на карточке устройства |

#### zigbeeModel ####
zigbeeModel - это массив идентификаторов модели, используемый в процессе интервью для поиска определения устройства по значению атрибута modelIdentifier базового кластера устройства.
Кроме идентификатора модели должно совпасть также наименование производителя (переменная vendor).
Если переменная zigbeeModel не задана, вместо нее используется значение переменной model.

#### "Отпечатки пальцев" устройства ####
Во многих случаях однозначный выбор определения устройства по значениям атрибутов manufacturerName и modelIdentifier базового кластера устройства невозможен. Например, под одним номером модели могут выпускаться устройства с различным количеством конечных точек или с отличающимся набором кластеров. В таких случаях для правильного выбора определения устройства применяются "отпечатки пальцев". Это массив структур, содержащих значения и комбинации значений тех атрибутов, которые позволяют различить похожие устройства.
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
В коде конвертеров могут встречаться "отпечатки пальцев", заданные через функции. Это сделано для лаконичности записи. Например в следующем примере идентификатор модели указан только один раз:
```
fingerprint: tuya.fingerprint('TS130F', ['_TZ3000_j1xl73iw', '_TZ3000_kmsbwdol', '_TZ3000_esynmmox', '_TZ3000_l6iqph4f', '_TZ3000_xdo0hj1k']),
```
Функция tuya.fingerprint() обогащает элементы массива по втором параметре значением первого параметра. В результате получается массив:
```
fingerprint: [{modelID: 'TS130F', manufacturerName: '_TZ3000_j1xl73iw'}, {modelID: 'TS130F', manufacturerName: '_TZ3000_kmsbwdol'}, {modelID: 'TS130F', manufacturerName: '_TZ3000_esynmmox'}, {modelID: 'TS130F', manufacturerName: '_TZ3000_l6iqph4f'}, {modelID: 'TS130F', manufacturerName: '_TZ3000_xdo0hj1k'}],
```
#### whiteLabel ####
Переменная whiteLabel - это массив, содержаший список брендов и марок, под которыми может выпускаться устройство. Указанные в whiteLabel значения могут не иметь ничего общего со значениями атрибутов manufacturerName и modelIdentifier базового кластера устройства.
Примеры использования whiteLabel:
```
whiteLabel: [
    {vendor: 'Frient', model: '94430', description: 'Smart Intelligent Smoke Alarm'},
    {vendor: 'Cavius', model: '2103', description: 'RF SMOKE ALARM, 5 YEAR 65MM'},
],
```
В составе whiteLabel может присутствовать fingerprint, для точного определения бренда и марки устройства:
```
whiteLabel: [
    {vendor: 'Schneider Electric', model: 'W599501', description: 'Wiser smoke alarm', fingerprint: [{modelID: 'W599501'}]},
    {vendor: 'Schneider Electric', model: '755WSA', description: 'Clipsal Wiser smoke alarm', fingerprint: [{modelID: '755WSA'}]},
],
```
#### exposes ####
exposes - это массив объектов, на основе которого формируется пользовательский состав устройства - набор элементов управления на странице "Свойства" в консоли Z2M и список сущностей, пробрасываемых через MQTT Discovery в Home Assistant или другое совместимое с MQTT Discovery программное обеспечение.
В exposes может быть включены предопределенные элементы, такие как выключатели, лампочки, датчики температуры, устройства климата, так и абстрактные элементы, такие как числовое значение, выбор из списка значений и т.п.
Полный список классов exposes можно найти в модуле /lib/exposes.ts.
Каждому элементу exposes соответствует одно или несколько именованных свойств (feature). Для предопределенных exposes имена свойств заданы в модуле exposes.ts, для абстрактных exposes имя свойства указывается в параметрах функции-конструктора.
Примеры exposes:
```
e.climate().withLocalTemperature().withSetpoint('occupied_heating_setpoint', 15, 35, 0.5).withSystemMode(['off', 'heat'])
    .withRunningMode(['off', 'heat']).withLocalTemperatureCalibration(-3.0, 3.0, 0.1).withEndpoint('l3'),
e.temperature(), e.humidity(), e.co2(),
exposes.numeric('low_pm25', ea.STATE_SET).withUnit('ppm').withDescription('Setting Low PM2.5 Border')
    .withValueMin(0).withValueMax(1000)],
exposes.binary('forced_recalibration', ea.STATE_SET, 'ON', 'OFF')
    .withDescription('Start FRC (Perform Forced Recalibration of the CO2 Sensor)'),
```
#### configure ####
После успешного завершения интервью выполняется конфигурирование устройства. Для этого вызывается функция конфигурирования, заданная в переменной configure.
Наиболее частыми действиями по конфигурированию являются:
- Биндинг.
- Настройка периодических отчетов. Концепция Zigbee подразумевает настройку периодических отчетов для атрибутов с признаком Reportable, выполняемую либо с координатора, либо с устроства. Чаще всего отчеты настраиваются на координаторе. В Z2M за это отвечает функция конфигурирования.
- Чтение значений атрибутов. Значения атрибутов, которые реализованы в устройстве без признака Reportable не могут быть отправлены координатору по инициативе устройства. Для таких атрибутов функция конфигурирования содержит вызов функции чтения значения атрибута.

#### fromZigbee ####
Переменная fromZigbee содержит массив объектов-конвертеров, содержащих логику преобразования значений атрибутов и команд кластеров устройства в операции публикации в топики MQTT.
В список fromZigbee могут включаться как стандартные конвертеры из модуля fromZigbee.ts, так и локальные, описанные в модуле внешнего конвертера.
Объект конвертера fromZigbee должен содержать переменные:
- cluster - Имя кластера, для которого применяется данный конвертер.
- type - Тип операции ZCL, для которой применяется данный конвертер. Чаще всего это операции 'attributeReport' и 'readResponse'.
- options - ???
- convert - Функция преобразования.
Результат функции представляет собой объект, содержаший значения свойств (features) exposes.
Для каждой операции, тип которой соответствует списку в переменной type, и кластера, указанного в переменной cluster, вызывается функция преобразования. Если таких конвертеров окажется несколько, они применяются по очереди и результаты преобразования объединяется в один объект.

#### toZigbee ####
Переменная toZigbee содержит массив объектов-конвертеров, содержащих логику преобразования событий топиков MQTT в операции чтения/записи значений атрибутов и выполнения команд кластеров устройства.
В список toZigbee могут включаться как стандартные конвертеры из модуля toZigbee.ts, так и локальные, описанные в модуле внешнего конвертера.
Объект конвертера toZigbee должен содержать переменные:
- key - Список имен свойств (features) exposes, для которых применяется данный конвертер.
- convertSet - Функция преобразования для операций установки значений свойств.
- convertGet - Функция преобразования для операций получения значений свойств.
Функция convertSet обычно отправляет на координатор команду записи значений определенных атрибутов кластера. Возвращаемым результатом функции convertSet является объект с новыми значениями свойств (features) exposes.
Функция convertGet реализует получение значений свойств, перечисленных в переменной key. Обычно в convertGet вызывается функция entity.read() для отправки на координатор команды записи значений атрибутов кластера. Вызов выполняется с ожидаением результата (await). Возвращаемое значение не предусмотрено.

#### onEvent ####
Переменная onEvent содержит функцию обработки низкоуровневых событий Z2M - завершения интервью устройства, остановки Z2M и др.
Пример функции onEvent:
```
onEvent: async (type, data, device, options, state) => {
    /**
     * The DIN power consumption module loses the configure reporting
     * after device restart/powerloss.
     * We reconfigure the reporting at deviceAnnounce.
     */
    if (type === 'deviceAnnounce') {
        for (const endpoint of device.endpoints) {
            for (const c of endpoint.configuredReportings) {
                await endpoint.configureReporting(c.cluster.name, [{
                    attribute: c.attribute.name, minimumReportInterval: c.minimumReportInterval,
                    maximumReportInterval: c.maximumReportInterval, reportableChange: c.reportableChange,
                }]);
            }
        }
    }
},
```

#### meta ####
Переменная meta содержит информацию об устройстве, которая используется в коде конвертора. Чаще всего переменная meta содержит атрибут multiEndpoint (false/true), в зависимости от значения которого формируются имена свойств в результатах функций преобразования.

Для устройств экосистемы Tuya переменная meta может содержать атрибут tuyaDatapoints. Массив tuyaDatapoints содержит комбинации номера dataPoint, имени свойства (feature) и вызов специализированной функции-конвертера значений точек данных Tuya. Данная возможность используется нея для всех описаний устройств Tuya. В настоящий момент считается устаревающей и заменяется на технологию modernExtend (см. далее).

#### endpoint ####
Переменная endpoint содержит функцию, возвращающую структуру маппинга имен конечных точек устройства и номеров конечных точек. 
Примеры:
```
```
Эта информация используется для формирования правильных имен свойств в функциях преобразования конвертера. Имена конечных точек в функции endpoint и имена конечных точек в определениях exposes должны соответствовать друг другу.
Пример кода, использующего значения meta и endpoint:
```
convert: (model, msg, publish, options, meta) => {
    const multiEndpoint = model.meta && model.meta.multiEndpoint;
    const property = multiEndpoint ? postfixWithEndpointName('state', msg, model, meta) : 'state';
    return {[property]: msg.data.presentValue ? 'ON' : 'OFF'};
},
```
Если устройство поддерживает несколько конечных точек (meta.multiEndpoint=true), вызовом функции postfixWithEndpointName() в имя свойства 'state' добавляется окончание, соответствующий конечной точке, например 'state_l2'.

#### extend ####
Переменная extend содержит массив объектов-расширений, которые похожи на объекты exposes, но расширяют определение устройства необходимыми объектами-конвертерами и кодом конфигурирования устройства. При полном покрытии описания устройства через extend отпадает необходимость в наличии в описании устройства переменных configure, fromZigbee, toZigbee.
В большинстве случаев в переменной extend используются вызовы функций, возвращающих объекты расширений, в частности функции расширений из модуля modernExtend.ts.
По названию модуля modernExtend такие функции называют modernExtend или "современными расширениями".
Пример использования современных расширений в описании устройства:
```
const definition = {
    zigbeeModel: ['lumi.sens'],
    model: 'WSDCGQ01LM',
    vendor: 'Xiaomi',
    description: 'MiJia temperature & humidity sensor',
    extend: [
      // Modern extends are incapsulating fromZigbee, toZigbee, exposes and more.
      temperature(),
      humidity(),
      battery(),
    ],
};
```
В список extend могут включаться также вызовы функций, формирующие специальные виды расширений. Например такие вызовы часто используются в описаниях устройств экосистемы Tuya:  
```
extend: tuya.extend.light_onoff_brightness_colortemp_color({colorTempRange: [153, 500], noConfigure: true}),
extend: tuya.extend.switch({
    electricalMeasurements: true, electricalMeasurementsFzConverter: fzLocal.TS011F_electrical_measurement, powerOutageMemory: true, indicatorMode: true, childLock: true}),
```

#### Другие материалы ####
- [Официальная документация](https://www.zigbee2mqtt.io/advanced/support-new-devices/01_support_new_devices.html#_2-2-adding-converter-s-for-your-device)
- [Объяснение частей кода внешнего конвертера](converter_parts.md)

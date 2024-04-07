# Поддержка устройств в Z2M #
Добавление поддержки для ранее неподдерживаемых устройств в Z2M в целом является относительно несложной задачей. Хотя это конечно зависит от многих факторов - наличие в базе устройств Z2M аналогичных моделей, использование в устройстве нестандартных кластеров и атрибутов, наличие у пользователя знаний в ИТ вообще и знания JavaScript в частности.

Поддержка нового устройства в Z2M реализуется написанием внешнего конвертера на языке JavaScript. Внешний конвертер копируется в папку Z2M (рядом с файлов configuration.yaml) и регистрируется на странице Настройки>Внешние конвертеры (указывается имя файла без пути, но с расширением .js).

В Сети на момент написания данного текста отсутствуют исчерпывающие руководства по реализации конвертеров Z2M для неподдерживаемых устройств. Дело осложняется периодическими изменениями ядра конвертеров Z2M, требующими адаптации кода существующих конвертеров для новых версий. Поэтому основным материалом для освоения написания конвертеров является исходных код библиотек Z2M.

В данной статье делается попытка разъяснить некоторые ключевые моменты, понимание которых делает процесс написания конвертеров чуть легче.

# Конвертеры Z2M #
Конвертеры устройств Z2M выделены в отдельную библиотеку [zigbee-herdsman-converters](https://github.com/Koenkk/zigbee-herdsman-converters). Рекомендуется изучить структуру этой библиотеки. Также рекомендуется ознакомиться с механизмом [расширений Z2M](https://github.com/Koenkk/zigbee2mqtt/tree/master/lib/extension).

Код библиотек написан на языке TypeScript (файлы .ts), являющемся расширенной версией языка JavaScript. Внешние конвертеры пишутся на JavaScript, поэтому для заимствования частей конвертеров из библиотеки Z2M может понадобится некоторая адаптация кода.

## Файл конвертера ##

Файл конвертера содержит описание (definition) модели устройства, содержащее:
- описательные атрибуты - производитель, модель, описание модели, пиктограмма и т.п.
- "отпечаток пальцев" модели (fingerprint), использующийся для подбора конвертера для подключенного устройства
- массив "навыков" (exposes), управляющий представлением атрибутов устройства в UI Z2M и отображением в модели данных ПО Умного Дома (например, Home Assistant)
- массив функций конвертеров, выполняющих преобразование данных между форматом Zigbee Cluster Library и форматом топиков MQTT
- функции конфигурирования устройства и настройки репортинга 

Некоторые из перечисленных элементов используются только на этапе джойна устройств, другие задействованы при каждом обмене данными с устройством.

## Загрузка внешних конвертеров ##
Загрузка внешних конвертеров в память выполняется в коде расширения [externalConverters.ts](https://github.com/Koenkk/zigbee2mqtt/blob/master/lib/extension/externalConverters.ts).
```
import * as zhc from 'zigbee-herdsman-converters';
import * as settings from '../util/settings';
import {loadExternalConverter} from '../util/utils';

...
for (const file of settings.get().external_converters) {
            try {
                for (const definition of loadExternalConverter(file)) {
                    const toAdd = {...definition};
                    delete toAdd['homeassistant'];
                    zhc.addDefinition(toAdd);
                }
            } catch (error) {
                logger.error(`Failed to load external converter file '${file}' (${error.message})`);
                logger.error(
                    `Probably there is a syntax error in the file or the external converter is not ` +
                    `compatible with the current Zigbee2MQTT version`);
                logger.error(
                    `Note that external converters are not meant for long term usage, it's meant for local ` +
                    `testing after which a pull request should be created to add out-of-the-box support for the device`,
                );
            }
        }
```
Как видно из кода, список имен файлов внешних конвертеров извлекается из настроек Z2M, содержимое этих файлов загружается в память, после чего все определения, найденные в файле внешнего конвертера добавляются в коллекцию definitions объекта ZigbeeHerdsmanConverters. В процессе добавления определения из него извлекаются название модели и "отпечатки пальцев" (fingerprint), которые записываются в таблицу соответствия (lookup), используемую для подбора определения для подключенного устройства.

## Использование конвертеров при обмене данными ##
Логика использования внешних конвертеров для преобразования Zigbee-данных начинается в расширениях [publish.ts](https://github.com/Koenkk/zigbee2mqtt/blob/master/lib/extension/publish.ts) и [receive.ts](https://github.com/Koenkk/zigbee2mqtt/blob/master/lib/extension/receive.ts).
Например в расширении receive.ts присутствует код:
```
...
        const converters = data.device.definition.fromZigbee.filter((c) => {
            const type = Array.isArray(c.type) ? c.type.includes(data.type) : c.type === data.type;
            return c.cluster === data.cluster && type;
        });
        // Check if there is an available converter, genOta messages are not interesting.
        const ignoreClusters: (string | number)[] = ['genOta', 'genTime', 'genBasic', 'genPollCtrl'];
        if (converters.length == 0 && !ignoreClusters.includes(data.cluster)) {
            logger.debug(`No converter available for '${data.device.definition.model}' with ` +
                `cluster '${data.cluster}' and type '${data.type}' and data '${stringify(data.data)}'`);
            utils.publishLastSeen({device: data.device, reason: 'messageEmitted'},
                settings.get(), true, this.publishEntityState);
            return;
        }
```
Здесь в определении устройства (data.device.definition) выполняется поиск функции-конвертера по типу и кластеру. В случае ненахождения функции-конвертера выдается ошибка.

Далее все подходящие функции-конвертеры применяются для формирования строки данных (payload) в формате, используемом для публикации данных в MQTT:
```
        let payload: KeyValue = {};
        for (const converter of converters) {
            try {
                const convertData = {...data, device: data.device.zh};
                const options: KeyValue = data.device.options;
                const converted = await converter.convert(
                    data.device.definition, convertData, publish, options, meta);
                if (converted) {
                    payload = {...payload, ...converted};
                }
            } catch (error) /* istanbul ignore next */ {
                logger.error(`Exception while calling fromZigbee converter: ${error.message}}`);
                logger.debug(error.stack);
            }
        }
```
Похожий код содержится и в расширении publish.ts, отвечающем за отправкой данных из MQTT в сторону устройства через Zigbee-координатор. 

Вначале ищется устройство (или группа устройств) по его entity_id из имени топика MQTT.
```
       const re = this.zigbee.resolveEntity(parsedTopic.ID);
        if (re == null) {
            this.legacyLog({type: `entity_not_found`, message: {friendly_name: parsedTopic.ID}});
            logger.error(`Entity '${parsedTopic.ID}' is unknown`);
            return;
        }
```
Последующий код содержит некоторую магию Z2M, после чего выполняется поиск конвертера по совпадению имени атрибута и конечной точки Zigbee и применения найденного конвертера.
```
            const converter = converters.find((c) =>
                c.key.includes(key) && (!c.endpoint || c.endpoint == endpointName));
...
            const result = await converter.convertSet(localTarget, key, value, meta);
```
## Описание конвертеров ##
[Определение устройства во внешнем конверторе](device_definition.md)
[Объяснение кода внешнего конвертера Z2M](converter_parts.md)

## Написание внешнего конвертера ##

[Расширение существующего конвертера](https://github.com/Muk911/zigbee/blob/main/zigbee2mqtt/converters/expand-converter.md)

[Перехват сообщений от устройства](https://github.com/Muk911/zigbee/tree/main/zigbee2mqtt/extensions#%D1%80%D0%B0%D1%81%D1%88%D0%B8%D1%80%D0%B5%D0%BD%D0%B8%D0%B5-%D0%B4%D0%BB%D1%8F-%D0%BF%D0%B5%D1%80%D0%B5%D1%85%D0%B2%D0%B0%D1%82%D0%B0-%D1%81%D0%BE%D0%BE%D0%B1%D1%89%D0%B5%D0%BD%D0%B8%D0%B9)

[Тестирование конвертеров](convert-test.md)

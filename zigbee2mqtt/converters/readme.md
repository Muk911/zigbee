# Поддержка устройств в Z2M #
Добавление поддержки для ранее неподдерживаемых устройств в Z2M в целом является относительно несложной задачей. Хотя это конечно зависит от многих факторов - наличие в базе устройств Z2M аналогичных моделей, использование в устройстве нестандартных кластеров и атрибутов, наличие у пользователя знаний в ИТ вообще и знания JavaScript в частности.

Поддержка нового устройства в Z2M реализуется написанием внешнего конвертера на языке JavaScript. Внешний конвертер копируется в папку Z2M и регистрируется на странице Найстройки.

В Сети на момент написания данного текста отсутствуют исчерпывающие руководства по реализации конвертеров Z2M для неподдерживаемых устройств. Дело осложняется периодическими изменениями ядра конвертеров Z2M, требующими адаптации кода существующих конвертеров для новых версий. Поэтому основным материалом для освоения написания конвертеров является исходных код библиотек Z2M.

В данной статье делается попытка разъяснить некоторые ключевые моменты, понимание которых делает процесс написания конвертеров чуть легче.

# Конвертеры Z2M #
Конвертеры устройств Z2M выделены в отдельную библиотеку [zigbee-herdsman-converters](https://github.com/Koenkk/zigbee-herdsman-converters). Рекомендуется изучить структуру этой библиотеки. Также рекомендуется ознакомиться с механизмом [расширений Z2M](https://github.com/Koenkk/zigbee2mqtt/tree/master/lib/extension).

Код библиотек написан на языке TypeScript (расширение файлов .ts), являющемся расширенной версией языка JavaScript. Внешние конвертеры пишутся на JavaScript, поэтому для заимствования частей конвертеров из библиотеки Z2M может понадобится некоторая адаптация кода.

## Файл конвертера ##

Файл конвертера содержит описание (definition) модели устройства, содержащее:
- описательные атрибуты - производитель, модели, описание модели, пиктограмма и т.п.
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

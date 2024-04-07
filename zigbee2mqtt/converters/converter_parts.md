### Объяснение кода внешнего конвертера Z2M ###
```
const fz = require('zigbee-herdsman-converters/converters/fromZigbee');
const tz = require('zigbee-herdsman-converters/converters/toZigbee');
```
-- загрузка объектов стандартных конвертеров.

```
const exposes = require('zigbee-herdsman-converters/lib/exposes');
```
-- загрузка объектов стандартных exposes.

```
const e = exposes.presets;
const ea = exposes.access;
```
-- получение ссылок для лаконичности записи exposes.

```
const reporting = require('zigbee-herdsman-converters/lib/reporting');
```
-- загрузка функций настройки периодических отчетов.

```
const constants = require('zigbee-herdsman-converters/lib/constants');
const utils = require('zigbee-herdsman-converters/lib/utils');
```
-- загрузка констант и вспомогательных функций.


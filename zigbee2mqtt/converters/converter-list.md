# Получение списка загруженных конвертеров Z2M #

Расширение Z2M для отображения списка загруженных конвертеров. Расширение добавляется на странице "Расширения" Z2M. При старте расширения во всплывающих окнах отображаются модели устройств загруженных конвертеров.
```
import * as zhc from 'zigbee-herdsman-converters';
import * as settings from '../util/settings';
import {loadExternalConverter} from '../util/utils';

...
const zhc = require('zigbee-herdsman-converters');

class ConverterListExtension {
    constructor(zigbee, mqtt, state, publishEntityState, eventBus, settings, logger) {
        this.zigbee = zigbee;
        this.eventBus = eventBus;
        this.logger = logger;
    }

    async start() {
      //this.logger.info('***');
      let n = 0;
      for (const device of this.zigbee.devices(false)) { //zhc.definitions) {
        n = n + 1;   
        this.logger.info(`*** vendor='${device.definition.vendor}' model='${device.definition.model}'`);
      }
      //this.logger.info(n);
    }

    async stop() {
        this.eventBus.removeListeners(this.constructor.name);
    }
}

module.exports = ConverterListExtension;
```

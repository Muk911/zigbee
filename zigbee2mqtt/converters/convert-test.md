# Тестирование конвертеров #
## Тестирование загрузки конвертеров ##
Быстрое тестирование загрузки файла конвертера можно выполнить с помощью следующего расширения Z2M. Расширение добавляется на странице "Расширения" Z2M.
При старте расширения выполняется загрузка в память модуля конвертера. Если ошибок при загрузке не возникло, выводятся имена моделей из определения устройства.
```
const converterFileName = 'acmelec2.js';

const utils = require('../util/utils');

class ConverterTestExtension {
    constructor(zigbee, mqtt, state, publishEntityState, eventBus, settings, logger) {
        this.eventBus = eventBus;
        this.logger = logger;
    }

   async start() 
        for (const definition of utils.loadExternalConverter(converterFileName)) {
           this.logger.info(definition.model);   
        }
    }

    async stop() {
        this.eventBus.removeListeners(this);
    }
}

module.exports = ConverterTestExtension;
```

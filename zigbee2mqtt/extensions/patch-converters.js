const zhc = require('zigbee-herdsman-converters');
const legacy = require("zigbee-herdsman-converters/lib/legacy");

class ConverterPathExtension {
    constructor(zigbee, mqtt, state, publishEntityState, eventBus, settings, logger) {
        //this.zigbee = zigbee;
        this.eventBus = eventBus;
        this.logger = logger;
    }

    async start() {
        const definition = zhc.definitions.find((d) => d.model === 'BHT-002-GCLZB');
        if(definition) {
            try {
                const toAdd = {...definition};
                toAdd.fingerprint = [{modelID: 'TS0601', manufacturerName: '_TZE200_o9d1hdma'}];
                toAdd.model = 'AE-668D-Zigbee';
                toAdd.vendor = 'Acmelec';
                toAdd.description = 'Acmelec AE-668D Zigbee Thermostat !!!';
                toAdd.fromZigbee.oldConvert = toAdd.fromZigbee.convert;
                toAdd.fromZigbee[0].convert = (model, msg, publish, options, meta) => {
                    const dpValue = msg.data.dpValues[0];
                    const dp = dpValue.dp;
                    const value = legacy.getDataValue(dpValue);
                    let temperature;
                    switch (dp) {
                        case legacy.dataPoints.moesLocalTemp:
                            temperature = value & 1<<15 ? value - (1<<16) + 1 : value;
                            temperature = parseFloat(temperature.toFixed(1));
                            if (temperature < 100) {
                                return {local_temperature: parseFloat(temperature.toFixed(1))};
                            }
                            break;
                        default:
                            meta.device.definition.fromZigbee.oldConvert(model, msg, publish, options, meta);
                            
                    }
                }
                zhc.addDefinition(toAdd);
                //this.logger.info(`Success to add external converter.`);
            } catch (error) {
                this.logger.error(`Failed to add external converter.`);
            }  
        } 
        else {
           this.logger.error('Definition not found.');
        }
    }
    
    async stop() {
        this.eventBus.removeListeners(this);
    }
}

module.exports = ConverterPathExtension;

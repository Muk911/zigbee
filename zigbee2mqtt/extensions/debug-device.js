const deviceIeeeAddr = '0xa4c138c25cfc85a4';

class DeviceDebugExtension {
    constructor(zigbee, mqtt, state, publishEntityState, eventBus, settings, logger) {
        //logger.info('Loaded DeviceDebugExtension');
        this.mqttBaseTopic = settings.get().mqtt.base_topic;
        this.eventBus = eventBus;
        this.mqtt = mqtt;
        this.logger = logger;
    }

    onDeviceMessage(data) {
        if(data.device.ieeeAddr == deviceIeeeAddr) {
            const dpValues = data.data.dpValues;
            if(dpValues) {
                this.logger.info(`*** cluster='${data.cluster}', type='${data.type}', dpValues='${JSON.stringify(dpValues)}'`);
            }
            else {
                this.logger.info(`*** cluster='${data.cluster}', type='${data.type}'`);
            }
        }
    }
    
//    onMQTTMessage(data) {
//    }
    
    async start() {
        this.eventBus.onDeviceMessage(this, (data) => {
            this.onDeviceMessage(data);
        });
//        this.eventBus.onMQTTMessage(this, (data) => {
//            this.onMQTTMessage(data);
//        });
    }

    async stop() {
        this.eventBus.removeListeners(this);
    }
}

module.exports = DeviceDebugExtension;

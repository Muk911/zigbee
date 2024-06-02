const fz = require('zigbee-herdsman-converters/converters/fromZigbee');
const tz = require('zigbee-herdsman-converters/converters/toZigbee');
const exposes = require('zigbee-herdsman-converters/lib/exposes');
const reporting = require('zigbee-herdsman-converters/lib/reporting');
const utils = require('zigbee-herdsman-converters/lib/utils');
const e = exposes.presets;
const ea = exposes.access;

const fzLocal = {
    current_level: {
        cluster: 'genLevelCtrl',
        type: ['attributeReport', 'readResponse'],
        convert: (model, msg, publish, options, meta) => { 
            if (msg.data.hasOwnProperty('currentLevel')) { 
              //meta.logger.info(`convert: ${msg.data['currentLevel']}`);             
              const currentLevel = Number(msg.data['currentLevel']);
              return {current_level: currentLevel};          
            }
        },
    }
}

const tzLocal = {
    current_level: {
        key: ['current_level'], 
        convertSet: async (entity, key, value, meta) => {
            if (key === 'current_level') {
                //meta.logger.info(`convertSet: ${value}`); 
                const payload = {level: Number(value), transtime: 0};
                await entity.command('genLevelCtrl', 'moveToLevel', payload, utils.getOptions(meta.mapped, entity));
                return {state: {current_level: value}};       
            }
        },
        convertGet: async (entity, key, meta) => {
            //meta.logger.info(`convertGet ${key}`); 
            await entity.read('genLevelCtrl', ['currentLevel']);
        },
    },
};

const definition = {
    fingerprint: [{modelID: 'servo', manufacturerName: 'MEA'}],
    model: 'servo',
    vendor: 'MEA',
    description: 'MEA servo',
    fromZigbee: [fzLocal.current_level],
    toZigbee: [tzLocal.current_level],
    exposes: [e.numeric('current_level', ea.ALL).withValueMin(1).withValueMax(254).withValueStep(1)
                .withDescription('Servo position')],
    configure: async (device, coordinatorEndpoint, logger) => {
        const endpoint = device.getEndpoint(1);
        await reporting.bind(endpoint, coordinatorEndpoint, ['genLevelCtrl']);
        await reporting.brightness(endpoint); // уже реализовано для currentLevel
    },
};

module.exports = definition;
const fz = require('zigbee-herdsman-converters/converters/fromZigbee');
const tz = require('zigbee-herdsman-converters/converters/toZigbee');
const exposes = require('zigbee-herdsman-converters/lib/exposes');
const reporting = require('zigbee-herdsman-converters/lib/reporting');
const types = require('zigbee-herdsman-converters/lib/types');
const constants = require('zigbee-herdsman-converters/lib/constants');
const utils = require('zigbee-herdsman-converters/lib/utils');
const e = exposes.presets;
const ea = exposes.access;

const dataType = {
    uint16: 33
};

const fzLocal = {
    on_time: {
        cluster: 'genOnOff',
        type: ['attributeReport', 'readResponse'],
        convert: (model, msg, publish, options, meta) => {
            const result = {};
            const data = msg.data;
            if (data.hasOwnProperty('onTime')) {
                //meta.logger.info(`fz: onTime ${JSON.stringify(data)}`);
                result[utils.postfixWithEndpointName('on_time', msg, model, meta)] = data['onTime'] / 10;
            }
            return result;
        },
    },
}

const tzLocal = {
    on_off: {
        key: ['state', 'on_time'], 
        convertSet: async (entity, key, value, meta) => {
           if (key === 'state') {
                const state = value.toLowerCase();
                await entity.command('genOnOff', state, {}, utils.getOptions(meta.mapped, entity));
                if (state === 'toggle') {
                    const currentState = meta.state[`state${meta.endpoint_name ? `_${meta.endpoint_name}` : ''}`];
                    return currentState ? {state: {state: currentState === 'OFF' ? 'ON' : 'OFF'}} : {};
                } else {
                    return {state: {state: state.toUpperCase()}};
                }           
           }
           if (key === 'on_time') {
                let onTime = Math.round(utils.toNumber(value, key) * 10);
                await entity.write('genOnOff', {0x4001: {value: onTime, type: dataType.uint16}});
                return {state: {on_time: value}};
           }
        },
        convertGet: async (entity, key, meta) => {
            //meta.logger.info(`convertGet: ${key}`); 
            await entity.read('genOnOff', ['onOff', 'onTime']);
        },
    },
};

const definition = {
    fingerprint: [{modelID: 'valves', manufacturerName: 'MEA'}],
    model: 'valves',
    vendor: 'MEA',
    description: 'MEA valves',
    fromZigbee: [fz.ignore_basic_report, fz.on_off, fzLocal.on_time],
    toZigbee: [tzLocal.on_off],
    exposes: [
      e.switch().withEndpoint('l1').setAccess('state', ea.STATE_SET).withDescription('State 1'),
      e.numeric('on_time', ea.ALL).withEndpoint('l1').withValueMin(10).withValueMax(4800).withDescription('Duration 1'),
      e.switch().withEndpoint('l2').setAccess('state', ea.STATE_SET).withDescription('State 2'),
      e.numeric('on_time', ea.ALL).withEndpoint('l2').withValueMin(10).withValueMax(4800).withDescription('Duration 2'),
      e.switch().withEndpoint('l3').setAccess('state', ea.STATE_SET).withDescription('State 3'),
      e.numeric('on_time', ea.ALL).withEndpoint('l3').withValueMin(10).withValueMax(4800).withDescription('Duration 3'),
      e.switch().withEndpoint('l4').setAccess('state', ea.STATE_SET).withDescription('State 4'),
      e.numeric('on_time', ea.ALL).withEndpoint('l4').withValueMin(10).withValueMax(4800).withDescription('Duration 4'),
      e.switch().withEndpoint('l5').setAccess('state', ea.STATE_SET).withDescription('State 5'),
      e.numeric('on_time', ea.ALL).withEndpoint('l5').withValueMin(10).withValueMax(4800).withDescription('Duration 5'),
      e.switch().withEndpoint('l6').setAccess('state', ea.STATE_SET).withDescription('State 6'),
      e.numeric('on_time', ea.ALL).withEndpoint('l6').withValueMin(10).withValueMax(4800).withDescription('Duration 6'),
    ],
    meta: {multiEndpoint: true},
    endpoint: (device) => {
            return {'l1': 1, 'l2': 2, 'l3': 3, 'l4': 4, 'l5': 5, 'l6': 6};
    },
    configure: async (device, coordinatorEndpoint, logger) => {
        const ep1 = device.getEndpoint(1);
        const ep2 = device.getEndpoint(2);
        const ep3 = device.getEndpoint(3);
        const ep4 = device.getEndpoint(4);
        const ep5 = device.getEndpoint(5);
        const ep6 = device.getEndpoint(6);
        await reporting.bind(ep1, coordinatorEndpoint, ['genOnOff']);
        await reporting.bind(ep2, coordinatorEndpoint, ['genOnOff']);
        await reporting.bind(ep3, coordinatorEndpoint, ['genOnOff']);
        await reporting.bind(ep4, coordinatorEndpoint, ['genOnOff']);
        await reporting.bind(ep5, coordinatorEndpoint, ['genOnOff']);
        await reporting.bind(ep6, coordinatorEndpoint, ['genOnOff']);
        await reporting.onOff(ep1);
        await reporting.onOff(ep2);
        await reporting.onOff(ep3);
        await reporting.onOff(ep4);
        await reporting.onOff(ep5);   
        await reporting.onOff(ep6);             
        await ep1.read('genOnOff', ['onTime']);
        await ep2.read('genOnOff', ['onTime']);
        await ep3.read('genOnOff', ['onTime']);
        await ep4.read('genOnOff', ['onTime']);
        await ep5.read('genOnOff', ['onTime']);
        await ep6.read('genOnOff', ['onTime']);
    },  
};

module.exports = definition;
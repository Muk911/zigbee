const fz = require('zigbee-herdsman-converters/converters/fromZigbee');
const tz = require('zigbee-herdsman-converters/converters/toZigbee');
const exposes = require('zigbee-herdsman-converters/lib/exposes');
const reporting = require('zigbee-herdsman-converters/lib/reporting');
const types = require('zigbee-herdsman-converters/lib/types');
const constants = require('zigbee-herdsman-converters/lib/constants');
//const utils = require('zigbee-herdsman-converters/lib/utils');
const e = exposes.presets;
const ea = exposes.access;

const definition = {
    fingerprint: [{modelID: 'door-lock', manufacturerName: 'MEA'}],
    model: 'door-lock',
    vendor: 'MEA',
    description: 'MEA Door Lock',
    fromZigbee: [fz.lock],
    toZigbee: [tz.lock],
    exposes: [
        e.lock(),
        //e.pincode(),
        e.door_state(),
        //e.lock_action(),
        //e.lock_action_user(),
        e.enum('lock_mode', ea.ALL, ['auto_off_away_off', 'auto_on_away_off', 'auto_off_away_on', 'auto_on_away_on'])
          .withDescription('Lock-Mode of the Lock'),
        //e.enum('service_mode', ea.ALL, ['deactivated', 'random_pin_1x_use', 'random_pin_24_hours']).withDescription('Service Mode of the Lock'),
    ],
    configure: async (device, coordinatorEndpoint) => {
        const endpoint = device.getEndpoint(1);
        await reporting.bind(endpoint, coordinatorEndpoint, ['closuresDoorLock', 'genPowerCfg']);
        await reporting.lockState(endpoint);
        await endpoint.read('closuresDoorLock', ['lockState', 'soundVolume', 'doorState']);
    },
};

module.exports = definition;
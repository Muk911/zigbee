const fz = require('zigbee-herdsman-converters/converters/fromZigbee');
const tz = require('zigbee-herdsman-converters/converters/toZigbee');
const exposes = require('zigbee-herdsman-converters/lib/exposes');
const reporting = require('zigbee-herdsman-converters/lib/reporting');
const e = exposes.presets;
const ea = exposes.access;

const definition = {
    fingerprint: [{modelID: 'door-lock', manufacturerName: 'MEA'}],
    model: 'door-lock',
    vendor: 'MEA',
    description: 'MEA Door Lock',
    fromZigbee: [fz.lock, fz.lock_operation_event, fz.battery],
    toZigbee: [tz.lock],
    exposes: [
        e.lock(),
        e.door_state(),
    ],
    configure: async (device, coordinatorEndpoint) => {
        const endpoint = device.getEndpoint(1);
        await reporting.bind(endpoint, coordinatorEndpoint, ['closuresDoorLock']);
        await reporting.lockState(endpoint);
        await reporting.doorState(endpoint);
    },
};

module.exports = definition;
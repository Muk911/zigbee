const fz = require('zigbee-herdsman-converters/converters/fromZigbee');
const tz = require('zigbee-herdsman-converters/converters/toZigbee');
const exposes = require('zigbee-herdsman-converters/lib/exposes');
const reporting = require('zigbee-herdsman-converters/lib/reporting');
//const utils = require('zigbee-herdsman-converters/lib/utils');
const e = exposes.presets;
const ea = exposes.access;

const definition = {
    fingerprint: [{modelID: 'thermostat', manufacturerName: 'MEA'}],
    model: 'thermostat',
    vendor: 'MEA',
    description: 'MEA thermostat',
    fromZigbee: [fz.ignore_basic_report, fz.thermostat, fz.fan],
    toZigbee: [
        tz.thermostat_local_temperature,
        tz.thermostat_system_mode,
        tz.thermostat_occupied_heating_setpoint,
        tz.thermostat_running_state,
        tz.thermostat_local_temperature_calibration,
        tz.thermostat_outdoor_temperature,
        tz.fan_mode,
    ],
    exposes: [
        e.climate()
            .withLocalTemperature()
            .withSetpoint('occupied_heating_setpoint', 5, 30, 0.5)
            .withLocalTemperatureCalibration(-5, 5, 0.1)
            .withSystemMode(['off', 'heat'])
            .withRunningState(['idle', 'heat'], ea.STATE)
            .withFanMode(['low', 'medium', 'high', 'auto'], ea.STATE_SET),
        e.numeric('outdoor_temperature', ea.STATE_GET).withUnit('°C')
            .withDescription('Current temperature measured from the floor sensor'),
    ],
    configure: async (device, coordinatorEndpoint, logger) => {
        const endpoint = device.getEndpoint(1);
        await reporting.bind(endpoint, coordinatorEndpoint, ['hvacThermostat', 'hvacFanCtrl']);
        await reporting.thermostatTemperature(endpoint);
        await reporting.thermostatRunningState(endpoint);
        
        await endpoint.read('hvacThermostat', ['systemMode', 'localTemperatureCalibration', 'outdoorTemperature', //'localTemperature', 
            'occupiedHeatingSetpoint', 'runningState']);
        await endpoint.read('hvacFanCtrl', ['fanMode']);
    },
};

module.exports = definition;
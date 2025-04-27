//const zhc = require('zigbee-herdsman-converters');
const fz = require('zigbee-herdsman-converters/converters/fromZigbee');
const tz = require('zigbee-herdsman-converters/converters/toZigbee');
const exposes = require('zigbee-herdsman-converters/lib/exposes');
const constants = require('zigbee-herdsman-converters/lib/constants');
const reporting = require('zigbee-herdsman-converters/lib/reporting');
const e = exposes.presets;
const ea = exposes.access;

const fzLocal = {
    alert_mode: {
        cluster: "genMultistateValue",
        type: ["attributeReport", "readResponse"],
        convert: (model, msg, publish, options, meta) => {
            const result = {};
            result["alert_mode"] = msg.data.presentValue;
            return result;
        },
    }
}

const tzLocal = {
    alert_mode: {
        key: ["alert_mode"],
        convertSet: async (entity, key, value, meta) => {
            await entity.write("genMultistateValue", {presentValue: value});
            return {state: {alert_mode: value}};
        },
        convertGet: async (entity, key, meta) => {
            await entity.read("genMultistateValue", ["presentValue"]);
        },
    }
};

const definition = {
    fingerprint: [{modelID: 'alert-system', manufacturerName: 'MEA'}],
    model: 'alert-system',
    vendor: 'MEA',
    description: 'MEA Alert System',
    fromZigbee: [fz.ignore_basic_report, fzLocal.alert_mode],
    toZigbee: [tzLocal.alert_mode],
    exposes: [
        e.enum("alert_mode", ea.ALL, [0, 1, 2, 3]).withDescription("Alert mode"),
    ],
    configure: async (device, coordinatorEndpoint, logger) => {
        const endpoint = device.getEndpoint(1);
        await reporting.bind(endpoint, coordinatorEndpoint, ['genMultistateValue']);
        await endpoint.configureReporting('genMultistateValue', reporting.payload('presentValue', 0, constants.repInterval.HOUR, 0));      
        await endpoint.read('genMultistateValue', ['presentValue']);
    },
};

module.exports = definition;
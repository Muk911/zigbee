const zhc = require('zigbee-herdsman-converters');
const legacy = require("zigbee-herdsman-converters/lib/legacy");

const baseDefinition = zhc.definitions.find((d) => d.model === 'BHT-002-GCLZB');
const definition = {
  ...baseDefinition,
  fingerprint: [{modelID: 'TS0601', manufacturerName: '_TZE200_o9d1hdma'}],
  model: 'AE-668D-Zigbee',
  vendor: 'Acmelec',
  description: 'Acmelec AE-668D Zigbee Thermostat',
};
const convertBase = definition.fromZigbee[0].convert;
definition.fromZigbee[0] = {
    ...definition.fromZigbee[0],
    convert: (model, msg, publish, options, meta) => {
        const dpValue = msg.data.dpValues[0]; 
        if(dpValue.dp == legacy.dataPoints.moesLocalTemp) {
            const value = legacy.getDataValue(dpValue);
            let temperature = value & 1<<15 ? value - (1<<16) + 1 : value;
            temperature = parseFloat(temperature.toFixed(1));
            if (temperature < 100) {
                return {local_temperature: parseFloat(temperature.toFixed(1))};
            }
        }
        else {
            return convertBase(model, msg, publish, options, meta);
            //return legacy.fz.moes_thermostat.convert(model, msg, publish, options, meta);
        }
    }   
}
module.exports = definition;

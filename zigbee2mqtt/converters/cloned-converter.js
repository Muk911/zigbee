const zhc = require('zigbee-herdsman-converters');

const baseDefinition = zhc.definitions.find((d) => d.model === 'BHT-002-GCLZB');
const definition = {
  ...baseDefinition,
  fingerprint: [{modelID: 'TS0601', manufacturerName: '_TZE200_o9d1hdma'}],
  //model: 'AE-668D-Zigbee',
  //vendor: 'Acmelec',
  //description: 'Acmelec AE-668D Zigbee Thermostat',
};
module.exports = definition;

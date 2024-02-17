https://t.me/zigbeat/2222

https://t.me/zigbeer/369018

const findByModel = require('zigbee-herdsman-converters').findByModel;
const tuya = require('zigbee-herdsman-converters/lib/tuya');

const oldDefinition = findByModel('is-thpl-zb');
const definition = {
    ...oldDefinition,
    fingerprint: tuya.fingerprint('TS0202', ['_TZ3210_oekbi7o4']),
};

module.exports = definition;

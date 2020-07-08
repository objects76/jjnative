

module.exports = require('bindings')('jjnative.node');

function numberFromBuffer(buf) {

    let number = 0;
    for (let i = 0; i < buf.length; i += 4) {
        number += buf.readUInt32LE(i) << (i * 8);
    }
    return number;
}

module.exports.util = {
    numberFromBuffer
};



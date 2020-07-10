

module.exports = require('bindings')('jjnative.node');


function bigintFromHandle(buf)  // handle:Buffer
{
    //return buf.readBigUInt64LE(buf);
    let n = BigInt(buf.readUInt32LE(0));

    if (buf.byteLength >= 8) {
        n += BigInt(buf.readUInt32LE(4)) * BigInt(0x100000000);
    }

    return n;
}


module.exports.util = {
    bigintFromHandle
};



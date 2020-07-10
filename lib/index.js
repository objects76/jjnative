

module.exports = require('bindings')('jjnative.node');


function bigintFromHandle(handle)  // handle:Buffer
{
    //return buf.readBigUInt64LE(buf);
    let n = BigInt(handle.readUInt32LE(0));

    if (handle.byteLength >= 8) {
        n += BigInt(handle.readUInt32LE(4)) * BigInt(0x100000000);
    }

    return n;
}


module.exports.util = {
    bigintFromHandle
};



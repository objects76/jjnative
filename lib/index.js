

module.exports = require('bindings')('jjnative.node');


function bigintFromHandle(handle)  // handle:Buffer
{
    if (handle.length >= 8)
        return handle.readBigUInt64LE(0);
    return BigInt(handle.readUInt32LE(0));
}


module.exports.util = {
    bigintFromHandle
};



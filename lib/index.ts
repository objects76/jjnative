module.exports = require("bindings")("jjnative.node");

function bigintFromHandle(handle: Buffer) {
  // handle:Buffer
  if (handle.length >= 8) return handle.readBigUInt64LE(0);
  return BigInt(handle.readUInt32LE(0));
}

const util = {
  bigintFromHandle,
};

module.exports.util = util;

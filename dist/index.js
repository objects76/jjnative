"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.bigintFromHandle = void 0;
function bigintFromHandle(handle) {
    // handle:Buffer
    //return buf.readBigUInt64LE(buf);
    let n = BigInt(handle.readUInt32LE(0));
    if (handle.byteLength >= 8) {
        n += BigInt(handle.readUInt32LE(4)) * BigInt(0x100000000);
    }
    return n;
}
exports.bigintFromHandle = bigintFromHandle;
exports.default = require("bindings")("jjnative.node");
//# sourceMappingURL=index.js.map
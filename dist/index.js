"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.bigintFromHandle = void 0;
const bindings_1 = __importDefault(require("bindings"));
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
exports.default = bindings_1.default("jjnative.node");

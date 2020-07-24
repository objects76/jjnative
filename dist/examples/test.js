"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    Object.defineProperty(o, k2, { enumerable: true, get: function() { return m[k]; } });
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || function (mod) {
    if (mod && mod.__esModule) return mod;
    var result = {};
    if (mod != null) for (var k in mod) if (k !== "default" && Object.hasOwnProperty.call(mod, k)) __createBinding(result, mod, k);
    __setModuleDefault(result, mod);
    return result;
};
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const process_1 = __importDefault(require("process"));
const Logger_1 = require("./Logger");
const index_1 = __importStar(require("../index"));
Logger_1.overrideConsole(true);
function test_main() {
    console.log("node version = ", process_1.default.versions.node);
    console.log("node_env=", process_1.default.env.NODE_ENV);
    let buf8 = Buffer.from([0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0x00, 0xab]);
    let buf4 = Buffer.from([0x00, 0x00, 0x00, 0xff]);
    console.log(buf8, "0x" + index_1.bigintFromHandle(buf8).toString(16));
    console.log(buf4, "0x" + index_1.bigintFromHandle(buf4).toString(16));
    try {
        // startKeyMonitor with invalid window handle.
        index_1.default.startKeyMonitor(BigInt(1));
    }
    catch (error) {
        console.error("startKeyMonitor exception: ", error);
    }
}
//test_main();
async function test_native() {
    index_1.default.init(console.log_native, null);
    //addon.init(null, null);
    // passing array test
    {
        const array = new Int32Array([1, 2, 3, 4, 5, 6, 7, 8, 9, 10]);
        try {
            console.log("ArrayBufferArgument=", index_1.default.ArrayBufferArgument(array.buffer));
            console.log("ArrayBufferArgument=", index_1.default.ArrayBufferArgument(array.buffer));
        }
        catch (error) {
            console.error(error);
        }
        return;
    }
    // promise test.
    console.log("result=", await index_1.default.getPrimeAsync());
    console.log(index_1.default.getPrimeSync());
    // class from addon.
    const obj = new index_1.default.NativeAddon(() => { console.log('Hi! I\'m a JSRef function!'); }, () => { console.log('Hi! I\'m a JS function!'); });
    // static member funcion test.
    index_1.default.NativeAddon.staticMethod1(obj, "using staticMethod1");
    try {
        index_1.default.NativeAddon.staticMethod_Invalid(obj, "using staticMethod_Invalid");
    }
    catch (error) {
        console.error(error);
    }
    // member function test.
    console.log(obj.tryCallByStoredReference());
    try {
        //console.log(obj.tryCallByStoredFunction());
    }
    catch (err) {
        console.error(err);
        console.error('This code crashes because JSFn is valid only inside the constructor function.');
        console.error('The lifespan of the JSFn function is limited to the execution of the constructor function.');
        console.error('After that, the function stored in JSFn is ready to be garbage collected and it cannot be used anymore.');
    }
}
test_native();

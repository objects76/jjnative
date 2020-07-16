

//const addon = require('../dist/index.js');
const addon = require("bindings")("jjnative.node");
const logger = require('./Logger');

function getObjectInformation(obj) {

    function getFuncs(obj, predefined) {
        let properties = new Set();
        for (let cur = obj; cur; cur = Object.getPrototypeOf(cur)) {
            const names = Object.getOwnPropertyNames(cur);
            for (const name of names) {
                if (!predefined.has(name))
                    properties.add(name);
            }
        };
        return ([...properties.keys()].filter((item) => typeof obj[item] === "function")).join('\n    ')
    }

    const predefined = new Set([
        'name', 'arguments', 'caller', 'prototype', 'arguments', 'apply', 'bind', 'call',
        '__defineGetter__', '__defineSetter__', 'hasOwnProperty', '__lookupGetter__', '__lookupSetter__', 'isPrototypeOf', 'propertyIsEnumerable',
        'toString', 'toLocalString'
    ]);

    const memfuncs = getFuncs(obj, predefined);

    const cls = obj.constructor;
    const staticfuncs = getFuncs(cls, predefined);
    return `[${cls.name}]\nmemberfunc:\n    ${memfuncs}\nstaticfunc:\n    ${staticfuncs}`;
}


function getStaticMethods(cls) {
    let properties = new Set();
    for (let currentObj = obj; currentObj; currentObj = Object.getPrototypeOf(currentObj)) {
        Object.getOwnPropertyNames(currentObj).map((item) => properties.add(item));
    }

    const functions = [...properties.keys()].filter((item) => typeof obj[item] === "function");
    return ['functions:', ...functions].join(',\n');
}
function test_main() {
    logger.log('node version = ', process.versions.node);
    logger.log('node_env=', process.env.NODE_ENV);
    //logger.log(addon);

    if (false && "getHandle test") {
        let buf8 = Buffer.from([0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0x00, 0xab]);
        let buf4 = Buffer.from([0x00, 0x00, 0x00, 0xff]);
        logger.log(buf8, '0x' + addon.bigintFromHandle(buf8).toString(16));
        logger.log(buf4, '0x' + addon.bigintFromHandle(buf4).toString(16));
        logger.log("addon=", getStaticMethods(addon));
    }

    const obj = new addon.NativeAddon(
        () => { console.log('Hi! I\'m a JSRef function!') },
        () => { console.log('Hi! I\'m a JS function!') }
    );
    logger.log(logger.getObjectInfo(obj));
    // logger.log(getObjectInformation(obj.constructor));

    //logger.log(obj);

    const clsNativeAddon = addon.NativeAddon;//obj.constructor;
    logger.log(logger.getObjectInfo(clsNativeAddon));

    clsNativeAddon.NativeAddon_dump1(obj);
}

// try {
//     // startKeyMonitor with invalid window handle.
//     jjnative.startKeyMonitor(1n);
// } catch (error) {
//     console.error('startKeyMonitor exception: ', error);
// }


// try {
//     // startKeyMonitor with not BigInt obj.
//     jjnative.startKeyMonitor(1);
// } catch (error) {
//     console.error('startKeyMonitor exception: ', error);
// }

test_main();


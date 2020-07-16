

const addon = require('../dist/index.js');
const logger = require('./Logger');
const getMethods = logger.getMethods;
const getStaticMethods = logger.getStaticMethods;

function test_main() {
    logger.log('node version = ', process.versions.node);
    logger.log('node_env=', process.env.NODE_ENV);
    logger.log(addon);

    let buf8 = Buffer.from([0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0x00, 0xab]);
    let buf4 = Buffer.from([0x00, 0x00, 0x00, 0xff]);
    logger.log(buf8, '0x' + addon.bigintFromHandle(buf8).toString(16));
    logger.log(buf4, '0x' + addon.bigintFromHandle(buf4).toString(16));

    logger.log("addon=", getStaticMethods(addon));

    const obj = new addon.NativeAddon(
        () => { console.log('Hi! I\'m a JSRef function!') },
        () => { console.log('Hi! I\'m a JS function!') }
    );
    logger.log(obj);
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




const jjnative = require('../lib/index');

console.log('node version = ', process.versions.node);
console.log('node_env=', process.env.NODE_ENV);

let buf8 = Buffer.from([0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0x00, 0xab]);
let buf4 = Buffer.from([0x00, 0x00, 0x00, 0xff]);
console.log(buf8, '0x' + jjnative.util.bigintFromHandle(buf8).toString(16));
console.log(buf4, '0x' + jjnative.util.bigintFromHandle(buf4).toString(16));

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



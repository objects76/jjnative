

const jjnative = require('../lib/index');

function buffer_test() {
    var sample = '00000000011C2212';
    var buffer = new Buffer(sample, 'hex');
    console.log('parse=', parseInt(sample, 16));
    console.log(buffer);

    const num = buffer.readUIntLE(0, 6);
    console.log('number=', num.toString(16));
}

buffer_test();

const hwnd = Buffer.allocUnsafe(8);
hwnd.writeInt32LE(0x00000000011C2212);
console.log('hwnd=', hwnd);

console.log(jjnative);

console.log(jjnative.startKeyMonitor(hwnd));
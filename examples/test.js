

const jjnative = require('../lib/index');


const hwnd = Buffer.allocUnsafe(8);
hwnd.writeInt32LE(0x00000000011C2212);
console.log('hwnd=', hwnd);

console.log(jjnative);

console.log(jjnative.startKeyMonitor(hwnd));
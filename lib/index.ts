interface IKeyboard {
  startKeyMonitor(hwnd: bigint): boolean;
  stopKeyMonitor(): boolean;
  pauseKeyMonitor(): boolean;
  resumeKeyMonitor(): boolean;
}

function bigintFromHandle(handle: Buffer) {
  // handle:Buffer
  //return buf.readBigUInt64LE(buf);
  let n = BigInt(handle.readUInt32LE(0));

  if (handle.byteLength >= 8) {
    n += BigInt(handle.readUInt32LE(4)) * BigInt(0x100000000);
  }

  return n;
}

// helper functions.
export default {
  bigintFromHandle,
};

const addon = {
  key: require("bindings")("jjnative.node") as IKeyboard,
};

export { addon };

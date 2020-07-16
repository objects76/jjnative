

// defined in addon(c++).
interface NativeAddon {
  tryCallByStoredReference(): string;
  tryCallByStoredFunction(): string;
  //static dump(inst: NativeAddon): void; // how to add static Native::dump(...) function.
}

// class for just type information.
// export class NativeAddon {
//   tryCallByStoredReference(): string { return "mockup code"; }
//   tryCallByStoredFunction(): string { return "mockup code"; }
//   static dump(inst: NativeAddon): void { console.error("static mockup"); }
// }

interface IAddon {
  // keyboard monitor in Windows.
  startKeyMonitor(hwnd: bigint): boolean;
  stopKeyMonitor(): boolean;
  pauseKeyMonitor(): boolean;
  resumeKeyMonitor(): boolean;

  // test
  getPrimeAsync(): Promise<number[]>;
  getPrimeSync(): number[];

  NativeAddon: {
    new(fnref: Function, fn: Function): NativeAddon
  }
  dumpNativeAddon(inst: NativeAddon): void;

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

export default require("bindings")("jjnative.node") as IAddon;

// helper functions.
export { bigintFromHandle };

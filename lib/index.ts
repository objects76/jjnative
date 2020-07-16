

// defined in addon(c++).
export interface NativeAddon {
  tryCallByStoredReference(): string;
  tryCallByStoredFunction(): string;
  //static staticMethod1(obj: NativeAddon, msg: string): void;
}
// export namespace NativeAddon {
//   export function staticMethod1(obj: NativeAddon, msg: string): void { console.log("skeleton"); }
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

  ArrayBufferArgument(buf: ArrayBuffer): number;
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

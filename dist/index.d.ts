/// <reference types="node" />
export interface NativeAddon {
    tryCallByStoredReference(): string;
    tryCallByStoredFunction(): string;
}
interface IAddon {
    startKeyMonitor(hwnd: bigint): boolean;
    stopKeyMonitor(): boolean;
    pauseKeyMonitor(): boolean;
    resumeKeyMonitor(): boolean;
    getPrimeAsync(): Promise<number[]>;
    getPrimeSync(): number[];
    NativeAddon: {
        new (fnref: Function, fn: Function): NativeAddon;
    };
    ArrayBufferArgument(buf: ArrayBuffer): number;
}
declare function bigintFromHandle(handle: Buffer): bigint;
declare const _default: IAddon;
export default _default;
export { bigintFromHandle };

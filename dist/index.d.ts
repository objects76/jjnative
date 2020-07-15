/// <reference types="node" />
interface INative {
    startKeyMonitor(hwnd: bigint): boolean;
    stopKeyMonitor(): boolean;
    pauseKeyMonitor(): boolean;
    resumeKeyMonitor(): boolean;
}
declare function bigintFromHandle(handle: Buffer): bigint;
declare const _default: {
    bigintFromHandle: typeof bigintFromHandle;
};
export default _default;
declare const addon: {
    key: INative;
};
export { addon };

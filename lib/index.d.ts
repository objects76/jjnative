//declare module "jjnative";

declare module "jjnative" {
  //export function numberFromBuffer(s: Buffer): number;
  export function startKeyMonitor(hwnd: number): bool;
  export function stopKeyMonitor(): bool;
  export function pauseKeyMonitor(): bool;
  export function resumeKeyMonitor(): bool;
}

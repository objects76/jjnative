# jjnative

```
{
  default: {
    startKeyMonitor: [Function],
    stopKeyMonitor: [Function],
    pauseKeyMonitor: [Function],
    resumeKeyMonitor: [Function],
    path: '...\\build\\Release\\jjnative.node'
    util {
      numberFromBuffer: [Function]
    }
  }
}

```

- startKeyMonitor(hwnd:number): bool;
- stopKeyMonitor(): bool;
- pauseKeyMonitor(): bool;
- resumeKeyMonitor(): bool;

- util.numberFromBuffer(buffer): number;

# example:

- examples/index.js (electron test)

# source path

- yarn add https://github.com/objects76/jjnative.git
- yarn add https://github.com/objects76/jjnative.git#<branch/commit/tag>

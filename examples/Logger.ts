import eleclog from "electron-log";

export function overrideConsole(isMain: boolean, logPath?: string) {
    if (logPath) {
        eleclog.transports.file.resolvePath = () => logPath;
        eleclog.transports.file.format = `{h}:{i}:{s}.{ms} ${isMain ? 'M' : 'R'}.[{level}] {text}`;
        console.log(eleclog.transports.file.getFile().path);
    }

    // impl
    function _get_caller() {
        const lines = new Error().stack?.split("\n");
        if (!lines) return "";
        const caller_line = lines[3];
        //    at new ExtractHooks (webpack:///./ExtractHooks.jsx?:93:5)
        const m = /at\s+([^(]+)((?!:\d).)+:(\d+)/.exec(caller_line);
        //console.log(caller_line);
        if (m!.length >= 4) {
            return `at ${(m![1] as any).trimEnd()}: ${m![3]} `;
        }
        return caller_line;
    }

    // replace console.
    (console as any).log_native = eleclog.log;
    console.log = (...args: any[]) => { eleclog.log(...args, _get_caller()); }
    console.error = (...args: any[]) => { eleclog.error(...args, _get_caller()); }
    console.warn = (...args: any[]) => { eleclog.warn(...args, _get_caller()); }
}

export function getMethods(obj: any) {
    let properties = new Set();
    let currentObj = obj;
    do {
        Object.getOwnPropertyNames(currentObj).map((item) => properties.add(item));
    } while ((currentObj = Object.getPrototypeOf(currentObj)));

    return [...properties.keys()].filter((value, index, array) => { return typeof value === "function"; });
};

export function getObjectInfo(obj: any) {

    function getFuncs(obj: any, predefined: Set<string>) {
        let properties = new Set();
        for (let cur = obj; cur; cur = Object.getPrototypeOf(cur)) {
            const names = Object.getOwnPropertyNames(cur);
            for (const name of names) {
                if (!predefined.has(name))
                    properties.add(name);
            }
        };
        return ([...properties.keys()].filter((value, index, array) => typeof value === "function")).join('\n    ')
    }

    const predefined = new Set([
        'name', 'arguments', 'caller', 'prototype', 'arguments', 'apply', 'bind', 'call',
        '__defineGetter__', '__defineSetter__', 'hasOwnProperty', '__lookupGetter__', '__lookupSetter__', 'isPrototypeOf', 'propertyIsEnumerable',
        'toString', 'toLocalString'
    ]);

    const memfuncs = getFuncs(obj, predefined);

    const cls = obj.constructor;
    const staticfuncs = getFuncs(cls, predefined);
    return `[${cls.name}]\nmemberfunc: \n    ${memfuncs} \nstaticfunc: \n    ${staticfuncs} `;
};

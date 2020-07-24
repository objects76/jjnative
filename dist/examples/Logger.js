"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.getObjectInfo = exports.getMethods = exports.overrideConsole = void 0;
const electron_log_1 = __importDefault(require("electron-log"));
function overrideConsole(isMain, logPath) {
    if (logPath) {
        electron_log_1.default.transports.file.resolvePath = () => logPath;
        electron_log_1.default.transports.file.format = `{h}:{i}:{s}.{ms} ${isMain ? 'M' : 'R'}.[{level}] {text}`;
        console.log(electron_log_1.default.transports.file.getFile().path);
    }
    // impl
    function _get_caller() {
        var _a;
        const lines = (_a = new Error().stack) === null || _a === void 0 ? void 0 : _a.split("\n");
        if (!lines)
            return "";
        const caller_line = lines[3];
        //    at new ExtractHooks (webpack:///./ExtractHooks.jsx?:93:5)
        const m = /at\s+([^(]+)((?!:\d).)+:(\d+)/.exec(caller_line);
        //console.log(caller_line);
        if (m.length >= 4) {
            return `at ${m[1].trimEnd()}: ${m[3]} `;
        }
        return caller_line;
    }
    // replace console.
    console.log_native = electron_log_1.default.log;
    console.log = (...args) => { electron_log_1.default.log(...args, _get_caller()); };
    console.error = (...args) => { electron_log_1.default.error(...args, _get_caller()); };
    console.warn = (...args) => { electron_log_1.default.warn(...args, _get_caller()); };
}
exports.overrideConsole = overrideConsole;
function getMethods(obj) {
    let properties = new Set();
    let currentObj = obj;
    do {
        Object.getOwnPropertyNames(currentObj).map((item) => properties.add(item));
    } while ((currentObj = Object.getPrototypeOf(currentObj)));
    return [...properties.keys()].filter((value, index, array) => { return typeof value === "function"; });
}
exports.getMethods = getMethods;
;
function getObjectInfo(obj) {
    function getFuncs(obj, predefined) {
        let properties = new Set();
        for (let cur = obj; cur; cur = Object.getPrototypeOf(cur)) {
            const names = Object.getOwnPropertyNames(cur);
            for (const name of names) {
                if (!predefined.has(name))
                    properties.add(name);
            }
        }
        ;
        return ([...properties.keys()].filter((value, index, array) => typeof value === "function")).join('\n    ');
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
}
exports.getObjectInfo = getObjectInfo;
;

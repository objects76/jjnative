const process = require('process');
const path = require('path');
// jjkim, 7/7/2010, initial

class Logger {

    enter(msg = '', use = true) {
        this.scope.push({ msg, use });
        if (use) {
            this.logger.log(this._header(), `{ ${msg} ${this._get_caller()}`);
        }
        ++this._level;
    }

    exit(obj = null) {
        --this._level;
        const { msg, use } = this.scope.pop();
        if (use) {
            this.logger.log(this._header(), `} ${msg} ${this._get_caller()}`);
        }
        return obj;
    }

    log(...args) {
        this.logger.log(this._header(), ...args, this._get_caller());
    }

    error(...args) {
        this.logger.error(this._header(), ...args, this._get_caller());
    }
    warn(...args) {
        this.logger.warn(this._header(), ...args, this._get_caller());
    }

    constructor(logPath, isMain) {
        this._level = 0;
        this.disable_scope = 0;
        this.scope = [];
        console.log('logfile=', logPath);
        try {
            this.logger = require('electron-log');
            if (logPath) {
                this.logger.transports.file.resolvePath = () => logPath;
                this.logger.transports.file.format = `{h}:{i}:{s}.{ms} ${isMain ? 'M' : 'R'}.[{level}] {text}`;
                console.log(this.logger.transports.file.getFile());
            }
        } catch (error) {
            this.logger = console;
            this.warn('set log file failed: ', error);
        }
    }

    //
    // private:
    //
    _header() {
        return " ".repeat(this._level * 4);
    }

    _get_caller() {
        const lines = new Error().stack.split("\n");
        const caller_line = lines[3];
        //    at new ExtractHooks (webpack:///./ExtractHooks.jsx?:93:5)
        const m = /at\s+([^(]+)((?!:\d).)+:(\d+)/.exec(caller_line);
        //console.log(caller_line);

        if (m.length >= 4) {
            return `at ${m[1].trimEnd()}: ${m[3]} `;
        }
        return caller_line;
    }
};

// // with babel
// const L = new Logger();
// export default L;


// // enum template.
// export function enum_xxx() {
//     let i = 0;
//     const code = {};
//     Object.defineProperties(code, {
//         code1: { value: i++, enumerable: true }, // define
//     });
//     return code;
// }

//module.exports = new Logger();
module.exports = Logger;

module.exports.getMethods = (obj) => {
    let properties = new Set();
    let currentObj = obj;
    do {
        Object.getOwnPropertyNames(currentObj).map((item) => properties.add(item));
    } while ((currentObj = Object.getPrototypeOf(currentObj)));

    return [...properties.keys()].filter((item) => typeof obj[item] === "function");
};

module.exports.getObjectInfo = (obj) => {

    function getFuncs(obj, predefined) {
        let properties = new Set();
        for (let cur = obj; cur; cur = Object.getPrototypeOf(cur)) {
            const names = Object.getOwnPropertyNames(cur);
            for (const name of names) {
                if (!predefined.has(name))
                    properties.add(name);
            }
        };
        return ([...properties.keys()].filter((item) => typeof obj[item] === "function")).join('\n    ')
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

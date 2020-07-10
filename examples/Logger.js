
// jjkim, 7/7/2010, initial
class Logger {

    enter(msg = '', use = true) {
        this.scope.push({ msg, use });
        if (use) {
            console.log(this._header(), `{ ${msg} ${this._get_caller()}`);
        }
        ++this._level;
    }

    exit(obj = null) {
        --this._level;
        const { msg, use } = this.scope.pop();
        if (use) {
            console.log(this._header(), `} ${msg} ${this._get_caller()}`);
        }
        return obj;
    }

    log(...args) {
        console.log(this._header(), ...args, `${this._get_caller()}`);
    }

    constructor() {
        this._level = 0;
        this.disable_scope = 0;
        this.scope = [];
        console.log('new Logger...');
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
        //    at LottoExtractHooks (webpack:///./LottoExtractHooks.jsx?:93:5)
        const m = /at\s+([^\s]+)((?!:\d).)+:(\d+)/.exec(caller_line);
        //console.log(caller_line);

        if (m) {
            return `at ${m[1]}:${m[3]}`;
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

const L = new Logger();
module.exports = L;

module.exports.getMethods = (obj) => {
    let properties = new Set();
    let currentObj = obj;
    do {
        Object.getOwnPropertyNames(currentObj).map((item) => properties.add(item));
    } while ((currentObj = Object.getPrototypeOf(currentObj)));
    return [...properties.keys()].filter((item) => typeof obj[item] === "function");
};
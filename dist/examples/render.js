"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const electron_1 = require("electron");
const Logger_1 = require("./Logger");
const index_1 = __importDefault(require("../index"));
Logger_1.overrideConsole(false, electron_1.remote.getGlobal('logPath'));
index_1.default.init(console.log_native, null);
function setup_keyevent() {
    document.addEventListener('keydown', (e) => {
        console.log(`${e.type}: ${e.key}, ${e.code}, ${e.keyCode}`, e);
    });
    document.addEventListener('keyup', (e) => {
        console.log(`${e.type}: ${e.key}, ${e.code}, ${e.keyCode}`, e);
    });
}
;
window.onload = function () {
    console.log('onload');
    setup_keyevent();
};

"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    Object.defineProperty(o, k2, { enumerable: true, get: function() { return m[k]; } });
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || function (mod) {
    if (mod && mod.__esModule) return mod;
    var result = {};
    if (mod != null) for (var k in mod) if (k !== "default" && Object.hasOwnProperty.call(mod, k)) __createBinding(result, mod, k);
    __setModuleDefault(result, mod);
    return result;
};
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const path_1 = __importDefault(require("path"));
const process_1 = __importDefault(require("process"));
const electron_1 = require("electron");
//
// app env setup.
//
(function () {
    const { app } = require('electron');
    const baseDir = path_1.default.join(app.getPath('documents'), 'elec-dir');
    const logDir = path_1.default.join(baseDir, 'logs');
    app.setPath('userData', baseDir);
    app.setPath('logs', logDir);
    const curtime = new Date().toLocaleString('en-US').replace(/, */, '-').replace(/[/: ]+/g, '_');
    global.logPath = path_1.default.join(logDir, `${app.getName()}-${curtime}-${process_1.default.pid}.log`);
})();
const electron_window_state_1 = __importDefault(require("electron-window-state"));
const electron_2 = require("electron");
const Logger_1 = require("./Logger");
const index_1 = __importStar(require("../index"));
Logger_1.overrideConsole(true, global.logPath);
index_1.default.init(console.log_native, null);
let mainWindow = null;
function createMainWindow() {
    let state = electron_window_state_1.default({});
    mainWindow = new electron_2.BrowserWindow({
        x: state.x, y: state.y,
        width: state.width, height: state.height,
        webPreferences: {
            nodeIntegration: true,
            enableRemoteModule: true,
        },
    });
    //win.loadURL('https://github.com');
    mainWindow.loadURL(`file://${__dirname}/index.html`);
    state.manage(mainWindow);
    mainWindow.webContents.openDevTools();
    mainWindow.setMenu(null);
    mainWindow.on('closed', () => {
        index_1.default.stopKeyMonitor();
    });
    mainWindow.on("focus", () => {
        index_1.default.resumeKeyMonitor();
        console.log('focused');
    });
    mainWindow.on("blur", () => {
        // const process = require('process');
        // const { crashReporter } = require('electron');
        // crashReporter.start({
        //     productName: 'iProduct',
        //     companyName: 'My Company, Inc.',
        //     submitURL: 'https://mycompany.sp.backtrace.io:6098/post?format=minidump&token=fff016fe152941145a880720158dbca39c0f1b524c96bbd7c95a896556284076',
        //     uploadToServer: false,
        // });
        // process.crash();
        index_1.default.pauseKeyMonitor();
        console.log('unfocused');
    });
    try {
        console.log('invalid start keymonitor');
        index_1.default.pauseKeyMonitor();
        index_1.default.stopKeyMonitor();
    }
    catch (error) {
        console.error(error);
    }
    try {
        const hwndNumber = index_1.bigintFromHandle(mainWindow.getNativeWindowHandle());
        index_1.default.startKeyMonitor(hwndNumber);
        console.log('start keymonitor');
    }
    catch (error) {
        console.error(error);
    }
}
electron_1.app.on('ready', createMainWindow);
electron_1.app.on('quit', (event, exitCode) => {
    console.log('quit:', exitCode);
});
electron_1.app.on('window-all-closed', () => {
    // Quit when all windows are closed - (Not macOS - Darwin)
    if (process_1.default.platform !== 'darwin')
        electron_1.app.quit();
});
// When app icon is clicked and app is running, (macOS) recreate the BrowserWindow
electron_1.app.on('activate', () => {
    if (mainWindow === null)
        createMainWindow();
});

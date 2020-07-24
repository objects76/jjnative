const fs = require('fs');
const path = require('path');
const process = require('process');

const { app } = require('electron');

//
// app env setup.
//
(function () {
    const { app } = require('electron');
    const baseDir = path.join(app.getPath('documents'), 'elec-dir');
    const logDir = path.join(baseDir, 'logs');
    app.setPath('userData', baseDir);
    app.setPath('logs', logDir);

    const curtime = new Date().toLocaleString('en-US').replace(/, */, '-').replace(/[/: ]+/g, '_');
    global.logPath = path.join(logDir, `${app.getName()}-${curtime}-${process.pid}.log`);
})();



const windowStateKeeper = require('electron-window-state')
const BrowserWindow = require('electron').BrowserWindow;

const Logger = require('./Logger');
const logger = new Logger(global.logPath, true);

const addonBase = require('../dist/index');
const addon = addonBase.default;

logger.log(addon);

function createMainWindow() {

    let state = windowStateKeeper({});

    const win = new BrowserWindow({
        x: state.x, y: state.y,
        width: state.width, height: state.height,
        webPreferences: {
            nodeIntegration: true,
        },
    });
    //win.loadURL('https://github.com');
    win.loadURL(`file://${__dirname}/index.html`);
    state.manage(win);

    win.webContents.openDevTools();
    win.setMenu(null);


    win.on('closed', () => {
        addon.stopKeyMonitor();
    });

    win.on("focus", () => {
        addon.resumeKeyMonitor();
        logger.log('focused');
    });
    win.on("blur", () => {
        // const process = require('process');
        // const { crashReporter } = require('electron');
        // crashReporter.start({
        //     productName: 'iProduct',
        //     companyName: 'My Company, Inc.',
        //     submitURL: 'https://mycompany.sp.backtrace.io:6098/post?format=minidump&token=fff016fe152941145a880720158dbca39c0f1b524c96bbd7c95a896556284076',
        //     uploadToServer: false,
        // });
        // process.crash();

        addon.pauseKeyMonitor();
        logger.log('unfocused');
    });

    try {
        logger.log('invalid start keymonitor');
        addon.pauseKeyMonitor();
        addon.stopKeyMonitor();
    } catch (error) {
        logger.error(error);
    }

    try {
        const hwndNumber = addonBase.bigintFromHandle(win.getNativeWindowHandle());
        addon.startKeyMonitor(hwndNumber);
        logger.log('start keymonitor');
    } catch (error) {
        logger.error(error);
    }
}

app.on('ready', createMainWindow);
app.on('quit', (event, exitCode) => {
    logger.log('quit:', exitCode);
});

app.on('window-all-closed', () => {
    // Quit when all windows are closed - (Not macOS - Darwin)
    if (process.platform !== 'darwin') app.quit()
})

// When app icon is clicked and app is running, (macOS) recreate the BrowserWindow
app.on('activate', () => {
    if (mainWindow === null) createMainWindow()
})


const windowStateKeeper = require('electron-window-state')
const { app, globalShortcut } = require('electron');
const BrowserWindow = require('electron').BrowserWindow;
const logger = require('./Logger');

const addonBase = require('../dist/index');
const addon = addonBase.default;

logger.log(addon);

function createMainWindow() {

    let state = windowStateKeeper({});

    const win = new BrowserWindow({
        x: state.x, y: state.y,
        width: state.width, height: state.height,
        webPreferences: {
            nodeIntegration: true
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
    });
    win.on("blur", () => {
        addon.pauseKeyMonitor();
    });

    try {
        addon.pauseKeyMonitor();
        addon.stopKeyMonitor();
    } catch (error) {
        logger.error(error);
    }

    try {
        const hwndNumber = addonBase.bigintFromHandle(win.getNativeWindowHandle());
        addon.startKeyMonitor(hwndNumber);
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



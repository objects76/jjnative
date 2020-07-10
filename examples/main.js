const { app, globalShortcut } = require('electron');
const BrowserWindow = require('electron').BrowserWindow;
const logger = require('./Logger');
const addon = require('../lib/index');

function createMainWindow() {
    const win = new BrowserWindow({
        webPreferences: {
            nodeIntegration: true
        },
    });
    //win.loadURL('https://github.com');
    win.loadURL(`file://${__dirname}/index.html`);


    win.webContents.openDevTools();
    win.setMenu(null);

    logger.log(addon);
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
        const hwndNumber = addon.util.bigintFromHandle(win.getNativeWindowHandle());
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



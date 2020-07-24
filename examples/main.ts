import path from 'path';
import process from 'process';
import { app } from 'electron';

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
    (global as any).logPath = path.join(logDir, `${app.getName()}-${curtime}-${process.pid}.log`);
})();



import windowStateKeeper from 'electron-window-state';
import { BrowserWindow } from 'electron';
import { overrideConsole } from "./Logger";
import addon, { bigintFromHandle } from '../index';

overrideConsole(true, (global as any).logPath);
addon.init((console as any).log_native, null);

let mainWindow: BrowserWindow | null = null;
function createMainWindow() {

    let state = windowStateKeeper({});

    mainWindow = new BrowserWindow({
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
        addon.stopKeyMonitor();
    });

    mainWindow.on("focus", () => {
        addon.resumeKeyMonitor();
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

        addon.pauseKeyMonitor();
        console.log('unfocused');
    });

    try {
        console.log('invalid start keymonitor');
        addon.pauseKeyMonitor();
        addon.stopKeyMonitor();
    } catch (error) {
        console.error(error);
    }

    try {
        const hwndNumber = bigintFromHandle(mainWindow.getNativeWindowHandle());
        addon.startKeyMonitor(hwndNumber);
        console.log('start keymonitor');
    } catch (error) {
        console.error(error);
    }
}

app.on('ready', createMainWindow);
app.on('quit', (event, exitCode) => {
    console.log('quit:', exitCode);
});

app.on('window-all-closed', () => {
    // Quit when all windows are closed - (Not macOS - Darwin)
    if (process.platform !== 'darwin') app.quit()
})

// When app icon is clicked and app is running, (macOS) recreate the BrowserWindow
app.on('activate', () => {
    if (mainWindow === null) createMainWindow()
})



import { remote } from 'electron';
import { overrideConsole } from "./Logger";
import addon from '../index';

overrideConsole(false, remote.getGlobal('logPath') as string);
addon.init((console as any).log_native, null);

function setup_keyevent() {
    document.addEventListener('keydown', (e) => {
        console.log(`${e.type}: ${e.key}, ${e.code}, ${e.keyCode}`, e);
    });

    document.addEventListener('keyup', (e) => {
        console.log(`${e.type}: ${e.key}, ${e.code}, ${e.keyCode}`, e);
    });
};

window.onload = function () {
    console.log('onload');
    setup_keyevent();
}


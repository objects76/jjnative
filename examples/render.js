
const remote = require('electron').remote;
const process = require('process');
const path = require('path');
const Logger = require('./Logger');


//const logger = new Logger(path.join(app.getPath('logs'), app.getName() + `-render-${process.pid}.log`));
const logger = new Logger(remote.getGlobal('logPath'), false);


function setup_keyevent() {
    document.addEventListener('keydown', (e) => {
        logger.log(`${e.type}: ${e.key}, ${e.code}, ${e.keyCode}`, e);
    });

    document.addEventListener('keyup', (e) => {
        logger.log(`${e.type}: ${e.key}, ${e.code}, ${e.keyCode}`, e);
    });
};

window.onload = function () {
    logger.log('onload');
    setup_keyevent();
}


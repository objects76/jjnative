const logger = require('./Logger');

logger.log('render');

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


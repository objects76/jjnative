const logger = require('./Logger');

logger.log('render');

function setup_keyevent() {
    document.addEventListener('keydown', (e) => {
        logger.log('keydown:', e);
    });

    document.addEventListener('keyup', (e) => {
        logger.log('keyup:', e);
    });
};

window.onload = function () {
    logger.log('onload');
    setup_keyevent();
}


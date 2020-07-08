const L = require('./Logger');

console.log('render');

function setup_keyevent() {
    document.addEventListener('keydown', (e) => {
        console.log('keydown:', e);
    });

    document.addEventListener('keyup', (e) => {
        console.log('keyup:', e);
    });
};

window.onload = function () {
    L.log('onload');
    setup_keyevent();
}


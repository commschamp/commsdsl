var assert = require('assert');
var factory = require('test33_emscripten.js');

function test1(instance) {
    console.log("test1");
    assert(instance.TEST33_MAJOR_VERSION == 1);
    assert(instance.TEST33_MINOR_VERSION == 2);
    assert(instance.TEST33_PATCH_VERSION == 3);
}

factory().then((instance) => {
    test1(instance);
});


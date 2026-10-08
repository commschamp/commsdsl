var assert = require('assert');
var factory = require('test11_emscripten.js');

function test1(instance) {
    console.log("test1");
    var msg11 = new instance.message_Msg11();

    try {
        msg11.field_f1().setBits(instance.message_Msg11Fields_F1_BitMask_M1);
        assert(msg11.field_f1().getBitValue_B0());
        assert(msg11.field_f1().getBitValue_B1());
    }
    finally {
        msg11.delete();
    }
}

factory().then((instance) => {
    test1(instance);
});


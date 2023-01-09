var assert = require('assert');
var factory = require('test1_emscripten.js');

function allocHandler(instance)
{
    var DerivedHandler = instance.MsgHandler.extend("MsgHandler", {
        handle_message_Msg1: function(msg) {
            this.msg1 = new instance.message_Msg1(msg);
        },
        handle_message_Msg2: function(msg) {
            this.msg2 = new instance.message_Msg2(msg);
        },
        handle_Message: function(msg) {
            assert(false); /* should not happen */
        }
    });    

    return new DerivedHandler;
}

function test1(instance) {
    var msg1 = new instance.message_Msg1();
    var frame = new instance.frame_Frame();
    var handler = allocHandler(instance);
    var buf = new instance.DataBuf();

    try {
        msg1.field_f1().setValue(123);
        msg1.field_f2().setValue(1);

        var es = frame.writeMessage(msg1, buf);
        assert(es == instance.comms_ErrorStatus.Success);
        frame.processInputData(buf, handler);
        assert(handler.msg1.field_f1().getValue() == handler.msg1.field_f1().getValue());
    }
    finally {
        buf.delete();
        handler.delete();
        frame.delete();
        msg1.delete();
    }

}

function test2(instance) {
    var jsArray = new Uint8Array([1, 1, 2, 3, 4]);
    var buf = instance.jsArrayToDataBuf(jsArray);
    var frame = new instance.frame_Frame();
    var handler = allocHandler(instance);

    try {
        frame.processInputData(buf, handler);
        assert(handler.msg1.field_f1().getValue() == 0x030201);
        assert(handler.msg1.field_f2().getValue() == 0x04);
    }
    finally {
        handler.delete();
        frame.delete();
        buf.delete();
    }
}

factory().then((instance) => {
    test1(instance);
    test2(instance);
});


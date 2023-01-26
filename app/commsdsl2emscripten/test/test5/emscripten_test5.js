var assert = require('assert');
var factory = require('test5_emscripten.js');

function allocHandler(instance)
{
    var DerivedHandler = instance.MsgHandler.extend("MsgHandler", {
        handle_message_Msg1: function(msg) {
            this.clean_Msg1();
            this.msg1 = new instance.message_Msg1(msg);
        },
        handle_Message: function(msg) {
            assert(false); /* should not happen */
        },
        clean_Msg1: function() {
            if (this.msg1) {
                this.msg1.delete();
            }
        },
        clean: function() {
            this.clean_Msg1();
        } 
    });    

    return new DerivedHandler;
}

function test1(instance) {
    console.log("!!! test1");
    var msg1 = new instance.message_Msg1();
    var frame = new instance.frame_Frame();
    var handler = allocHandler(instance);
    var buf = new instance.DataBuf();

    try {
        msg1.field_f1().ref().setValue(instance.field_E1_ValueType.V2);

        var es = frame.writeMessage(msg1, buf);
        console.log("Output buf: " + instance.dataBufMemoryView(buf));
        assert(es == instance.comms_ErrorStatus.Success);
        frame.processInputData(buf, handler);
        assert(instance.eq_message_Msg1(msg1, handler.msg1));
    }
    finally {
        buf.delete();
        handler.clean();
        handler.delete();
        frame.delete();
        msg1.delete();
    }
}

function test2(instance) {
    console.log("!!! test2");
    var jsArray = new Uint8Array([0x01, 0x00, 0x02, 0x05, 0x00, 0x0a, 0x03]);
    var buf = instance.jsArrayToDataBuf(jsArray);
    var frame = new instance.frame_Frame();
    var handler = allocHandler(instance);

    try {
        frame.processInputData(buf, handler);
        assert(handler.msg1.field_f1().ref().getValue() == instance.field_E1_ValueType.V2);
        assert(handler.msg1.field_f2().ref().getValue() == instance.MsgId.m5);
        assert(handler.msg1.field_f3().ref().getValue() == instance.field_E2_ValueType.V10);
        assert(handler.msg1.field_f4().getValue() == instance.message_Msg1Fields_F4_ValueType.V3);
    }
    finally {
        handler.clean();
        handler.delete();
        frame.delete();
        buf.delete();
    }
}

factory().then((instance) => {
    test1(instance);
    test2(instance);
});


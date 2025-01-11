var assert = require('assert');
var factory = require('test15_emscripten.js');

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
    console.log("test1");
    var msg1 = new instance.message_Msg1();
    var frame = new instance.frame_Frame();
    var handler = allocHandler(instance);
    var buf = new instance.DataBuf();

    try {
        assert(instance.dataBufMemoryView(msg1.field_f1().ref().getValue()), new Uint8Array([0x0a, 0x0b, 0x0c, 0x0d, 0x01]));
        assert(msg1.field_f10().doesExist());

        msg1.field_f1().ref().assignJsArray(new Uint8Array([0x01,0x02, 0x03, 0x00, 0x00]));
        msg1.field_f2().ref().assignJsArray(new Uint8Array([0x04, 0x05, 0x06]));
        msg1.field_f3().ref().assignJsArray(new Uint8Array([0x07, 0x08]));
        msg1.field_f10().field().assignJsArray(new Uint8Array([0x0a, 0x0b]));

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

factory().then((instance) => {
    test1(instance);
});


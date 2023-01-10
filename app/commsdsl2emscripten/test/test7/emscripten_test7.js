var assert = require('assert');
var factory = require('test7_emscripten.js');

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
        assert(msg1.field_f1().ref().getBitValue_b2());
        assert(!msg1.field_f1().ref().getBitValue_b3());

        msg1.field_f1().ref().setValue(0xff);
        msg1.field_f2().setBitValue_b3(true);

        assert(msg1.field_f2().getValue() == 0x8);

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
    var jsArray = new Uint8Array([0x01, 0x00, 0x00, 0x06, 0x00]);
    var buf = instance.jsArrayToDataBuf(jsArray);
    var frame = new instance.frame_Frame();
    var frameFields = new instance.frame_Frame_AllFields();
    var handler = allocHandler(instance);

    try {
        // frame.processInputData(buf, handler);
        frame.processInputDataSingleMsg(buf, handler, frameFields)
        assert(handler.msg1);
        assert(frameFields.getID().getValue(), 1);
        assert(instance.dataBufMemoryView(frameFields.getData().getValue()), new Uint8Array([0x00, 0x00, 0x06, 0x00]))
    }
    finally {
        handler.clean();
        handler.delete();
        frameFields.delete();
        frame.delete();
        buf.delete();
    }
}

factory().then((instance) => {
    test1(instance);
    test2(instance);
});


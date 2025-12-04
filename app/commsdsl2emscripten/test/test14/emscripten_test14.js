var assert = require('assert');
var factory = require('test14_emscripten.js');

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
        assert(msg1.field_f1().ref().getValue() == "hello");
        assert(msg1.field_f2().ref().getValue() == "hello");
        assert(msg1.field_f3().ref().getValue() == "hello");
        assert(msg1.field_f4().ref().getValue() == "");
        assert(msg1.field_f10().getValue() == "");

        msg1.field_f1().ref().setValue("bla");
        msg1.field_f2().ref().setValue("aaa");
        msg1.field_f3().ref().setValue("bbb");
        msg1.field_f4().ref().setValue("ccc");
        msg1.field_f10().setValue("ddd");

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


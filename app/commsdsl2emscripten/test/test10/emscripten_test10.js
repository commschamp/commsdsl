var assert = require('assert');
var factory = require('test10_emscripten.js');

function allocHandler(instance)
{
    var DerivedHandler = instance.MsgHandler.extend("MsgHandler", {
        handle_message_Msg1: function(msg) {
            this.clean_Msg1();
            this.msg1 = new instance.message_Msg1(msg);
        },
        handle_message_Msg2: function(msg) {
            this.clean_Msg2();
            this.msg2 = new instance.message_Msg2(msg);
        },
        handle_Message: function(msg) {
            assert(false); /* should not happen */
        },
        clean_Msg1: function() {
            if (this.msg1) {
                this.msg1.delete();
            }
        },
        clean_Msg2: function() {
            if (this.msg2) {
                this.msg2.delete();
            }
        },
        clean: function() {
            this.clean_Msg1();
            this.clean_Msg2();
        }
    });

    return new DerivedHandler;
}

function test1(instance) {
    console.log("test1");
    var msg2 = new instance.message_Msg2();
    var frame = new instance.frame_Frame();
    var handler = allocHandler(instance);
    var buf = new instance.DataBuf();

    try {
        msg2.field_f1().ref().field_year().setValue(2127);
        msg2.field_f1().ref().field_month().setValue(1);
        msg2.field_f1().ref().field_day().setValue(10);

        var es = frame.writeMessage(msg2, buf);
        console.log("Output buf: " + instance.dataBufMemoryView(buf));
        assert(es == instance.comms_ErrorStatus.Success);
        frame.processInputData(buf, handler);
        assert(instance.eq_message_Msg2(msg2, handler.msg2));
    }
    finally {
        buf.delete();
        handler.clean();
        handler.delete();
        frame.delete();
        msg2.delete();
    }
}

factory().then((instance) => {
    test1(instance);
});


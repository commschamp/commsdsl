var assert = require('assert');
var factory = require('test23_emscripten.js');

function allocHandler(instance)
{
    var DerivedHandler = instance.MsgHandler.extend("MsgHandler", {
        handle_message_Msg1: function(msg) {
            this.clean_Msg1();
            this.msg = new instance.message_Msg1(msg);
        },
        handle_message_Msg2: function(msg) {
            this.clean_Msg2();
            this.msg2 = new instance.message_Msg2(msg);
        },
        handle_message_Msg3: function(msg) {
            this.clean_Msg3();
            this.msg3 = new instance.message_Msg3(msg);
        },        
        handle_message_Msg4: function(msg) {
            this.clean_Msg4();
            this.msg4 = new instance.message_Msg4(msg);
        },        
        handle_Message: function(msg) {
            assert(false); /* should not happen */
        },
        clean_Msg1: function() {
            if (this.msg) {
                this.msg.delete();
            }
        },
        clean_Msg2: function() {
            if (this.msg2) {
                this.msg2.delete();
            }
        },   
        clean_Msg3: function() {
            if (this.msg3) {
                this.msg3.delete();
            }
        },  
        clean_Msg4: function() {
            if (this.msg4) {
                this.msg4.delete();
            }
        },                         
        clean: function() {
            this.clean_Msg1();
            this.clean_Msg2();
            this.clean_Msg3();
            this.clean_Msg4();
        } 
    });    

    return new DerivedHandler;
}

function test1(instance) {
    console.log("!!! test1");
    var msg = new instance.message_Msg4();
    var frame = new instance.frame_Frame();
    var handler = allocHandler(instance);
    var buf = new instance.DataBuf();

    try {
        assert(msg.field_b1().ref().field_data2().doesExist());

        var es = frame.writeMessage(msg, buf);
        console.log("Output buf: " + instance.dataBufMemoryView(buf));
        assert(es == instance.comms_ErrorStatus.Success);
        frame.processInputData(buf, handler);
        assert(instance.eq_message_Msg4(msg, handler.msg4));
    }
    finally {
        buf.delete();
        handler.clean();
        handler.delete();
        frame.delete();
        msg.delete();
    }
}

function test2(instance) {
    console.log("!!! test2");
    var msg = new instance.message_Msg4();
    var frame = new instance.frame_Frame();
    var handler = allocHandler(instance);
    var buf = new instance.DataBuf();

    try {
        assert(msg.field_b1().ref().field_data2().doesExist());
        msg.transportField_version().setValue(1);
        msg.refresh();
        assert(msg.field_b1().ref().field_data2().isMissing());

        var es = frame.writeMessage(msg, buf);
        console.log("Output buf: " + instance.dataBufMemoryView(buf));
        assert(es == instance.comms_ErrorStatus.Success);
        frame.processInputData(buf, handler);
        assert(instance.eq_message_Msg4(msg, handler.msg4));
    }
    finally {
        buf.delete();
        handler.clean();
        handler.delete();
        frame.delete();
        msg.delete();
    }
}

factory().then((instance) => {
    test1(instance);
    test2(instance);
});


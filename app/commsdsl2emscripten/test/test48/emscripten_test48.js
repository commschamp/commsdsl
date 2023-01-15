var assert = require('assert');
var factory = require('test48_emscripten.js');

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
        handle_message_Msg3: function(msg) {
            this.clean_Msg3();
            this.msg3 = new instance.message_Msg3(msg);
        },
        handle_message_Msg4: function(msg) {
            this.clean_Msg4();
            this.msg4 = new instance.message_Msg4(msg);
        },
        handle_message_Msg5: function(msg) {
            this.clean_Msg5();
            this.msg5 = new instance.message_Msg5(msg);
        },
        handle_message_Msg6: function(msg) {
            this.clean_Msg6();
            this.msg6 = new instance.message_Msg6(msg);
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
        clean_Msg5: function() {
            if (this.msg5) {
                this.msg5.delete();
            }
        },
        clean_Msg6: function() {
            if (this.msg6) {
                this.msg6.delete();
            }
        },               
        clean: function() {
            this.clean_Msg1();
            this.clean_Msg2();
            this.clean_Msg3();
            this.clean_Msg4();
            this.clean_Msg5();
            this.clean_Msg6();
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
        var f1Vec = msg.field_f1().value();
        f1Vec.resize(123);

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
    var f = new instance.field_Length();
    try {
        f.setValue(123);
        assert(f.getValue() == 123);
        assert(f.field_short().getValue() == 123);
        assert(f.field_long().isMissing());

        f.setValue(300);
        assert(f.getValue() == 300);
        assert(f.field_short().getValue() == 0xff);
        assert(f.field_long().doesExist());
        assert(f.field_long().field().getValue() == 300);        
    }
    finally {
        f.delete();
    }
}

function test3(instance) {
    console.log("!!! test3");
    var f = new instance.field_Length2();
    try {
        f.setValue(123);
        assert(f.getValue() == 123);
        assert(f.field_len().getValue() == 123);
    }
    finally {
        f.delete();
    }
}

factory().then((instance) => {
    test1(instance);
    test2(instance);
    test3(instance);
});


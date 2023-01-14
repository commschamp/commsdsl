var assert = require('assert');
var factory = require('test16_emscripten.js');

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
    console.log("!!! test1");
    var msg1 = new instance.message_Msg1();
    var frame = new instance.frame_Frame();
    var handler = allocHandler(instance);
    var buf = new instance.DataBuf();

    try {

        var f1Vec = msg1.field_f1().ref().value();
        f1Vec.resize(5);
        f1Vec.at(0).setValue(1);
        f1Vec.at(1).setValue(2);
        // The rest are defaulted
        
        var f2Vec = msg1.field_f2().ref().value();
        f2Vec.resize(3);
        f2Vec.at(0).setValue(0x3);
        f2Vec.at(1).setValue(0x4);
        f2Vec.at(2).setValue(0x5);

        var f3Vec = msg1.field_f3().ref().value();
        f3Vec.resize(2);
        f3Vec.at(0).setValue(0x6);
        f3Vec.at(1).setValue(0x7);
        
        assert(msg1.field_f4().doesExist());
        var f4Vec = msg1.field_f4().field().value();
        f4Vec.resize(2);
        f4Vec.at(0).setValue(0x8);
        f4Vec.at(1).setValue(0x9);          
        
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


var assert = require('assert');
var factory = require('test28_emscripten.js');

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

function allocVariant1Handler(instance)
{
    var DerivedHandler = instance.field_Variant1_Handler.extend("field_Variant1_Handler", {
        handle_p1: function(f) {
            this.clean_p1();
            this.p1 = new instance.field_Variant1Members_P1(f);
        },
        handle_p2: function(f) {
            this.clean_p2();
            this.p2 = new instance.field_Variant1Members_P2(f);
        },
        clean_p1: function() {
            if (this.p1) {
                this.p1.delete();
            }
        },
        clean_p2: function() {
            if (this.p2) {
                this.p2.delete();
            }
        },   
        clean: function() {
            this.clean_p1();
            this.clean_p2();
        } 
    });    

    return new DerivedHandler;
}

function test1(instance) {
    console.log("test1");
    var msg = new instance.message_Msg1();
    var frame = new instance.frame_Frame();
    var handler = allocHandler(instance);
    var vh = allocVariant1Handler(instance);
    var buf = new instance.DataBuf();

    try {
        assert(!msg.field_variant1().ref().valid())
        assert(msg.field_variant1().ref().currentField() == 2);
        msg.field_variant1().ref().initField_p2();
        assert(msg.field_variant1().ref().currentField() == 1);
        assert(msg.field_variant2().ref().currentField() == 0);

        var es = frame.writeMessage(msg, buf);
        console.log("Output buf: " + instance.dataBufMemoryView(buf));
        assert(es == instance.comms_ErrorStatus.Success);
        frame.processInputData(buf, handler);
        assert(instance.eq_message_Msg1(msg, handler.msg1));

        handler.msg1.field_variant1().ref().currentFieldExec(vh);   
        assert(instance.eq_field_Variant1Members_P2(vh.p2, msg.field_variant1().ref().accessField_p2()))         
    }
    finally {
        buf.delete();
        vh.clean();
        vh.delete();
        handler.clean();
        handler.delete();
        frame.delete();
        msg.delete();
    }
}

factory().then((instance) => {
    test1(instance);
});


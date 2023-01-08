var assert = require('assert');
var factory = require('test1_emscripten.js');

factory().then((instance) => {
    console.log("Loaded");

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

    var msg1 = new instance.message_Msg1();
    msg1.field_f1().setValue(123);
    msg1.field_f2().setValue(1);

    var frame = new instance.frame_Frame();

    var buf = new instance.DataBuf();
    var es = frame.writeMessage(msg1, buf);
    assert(es == instance.comms_ErrorStatus.Success);

    var handler = new DerivedHandler;
    frame.processInputData(buf, handler);

    assert(handler.msg1.field_f1().getValue() == handler.msg1.field_f1().getValue());

    handler.delete();
    buf.delete();
    frame.delete();
    msg1.delete();

//     var data = [1, 2, 3, 4, 5];
//     var data2 = new Uint8Array(data);


//     //var handler = new instance.Handler();
    
//     var msg1 = new instance.Msg1();
//     msg1.field_f1().value = 20;
//     //msg1.field_f2().value = data;
//     msg1.field_f2().setValue(data2);
//     var vecData = msg1.field_f2().getValue();

//     var fieldData = new Uint8Array(vecData.size()).map((_, id) => vecData.get(id))
//     console.log("!!!data: " + fieldData);

//     msg1.dispatch(handler);

//     msg1.delete();
//     handler.delete()

//     console.log("End");
});


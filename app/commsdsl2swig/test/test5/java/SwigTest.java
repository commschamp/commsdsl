public class SwigTest {
    private test5_message_Msg1 m_msg1 = null;

    static {
        System.loadLibrary("commsdsl2swig_test5_java");
    }

    public void setMsg1(test5_message_Msg1 msg) {
        m_msg1 = msg;
    }    

    public void doTest1() {
        var msg = new test5_message_Msg1();
        msg.field_f1().ref().setValue(test5_field_E1.ValueType.V2);
        msg.field_f2().ref().setValue(test5_MsgId.MsgId_m5);
        msg.field_f3().ref().setValue(test5_field_E2.ValueType.V10);
        msg.field_f4().setValue(test5_message_Msg1Fields_F4.ValueType.V3);

        var frame = new test5_frame_Frame();
        var buf = frame.writeMessage(msg);
        var handler = new TestMsgHandler(this);

        var consumed = frame.processInputData(buf, handler);
        assert consumed == buf.size();
        assert test5.eq_test5_message_Msg1(msg, m_msg1): "Messages not equal";
        System.out.println(m_msg1.field_f4().name() + " = " + m_msg1.field_f4().valueName());
        System.out.println("Test1 Complete");
    }

    public static void main(String argv[]) {
        var suite = new SwigTest();
        suite.doTest1();
    }
}
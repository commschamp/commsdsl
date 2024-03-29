public class SwigTest {
    private test5_swig.message_Msg1 m_msg1 = null;

    static {
        System.loadLibrary("test5_swig_java");
    }

    public void setMsg1(test5_swig.message_Msg1 msg) {
        m_msg1 = msg;
    }    

    public void doTest1() {
        var msg = new test5_swig.message_Msg1();
        msg.field_f1().ref().setValue(test5_swig.field_E1.ValueType.V2);
        msg.field_f2().ref().setValue(test5_swig.field_MsgId.ValueType.m5);
        msg.field_f3().ref().setValue(test5_swig.field_E2.ValueType.V10);
        msg.field_f4().setValue(test5_swig.message_Msg1Fields_F4.ValueType.V3);

        var frame = new test5_swig.frame_Frame();
        var buf = frame.writeMessage(msg);
        var handler = new TestMsgHandler(this);

        var consumed = frame.processInputData(buf, handler);
        assert consumed == buf.size();
        assert test5_swig.test5.eq_message_Msg1(msg, m_msg1): "Messages not equal";
        System.out.println(m_msg1.field_f4().name() + " = " + m_msg1.field_f4().valueName());
        System.out.println("Test1 Complete");
    }

    public static void main(String argv[]) {
        var suite = new SwigTest();
        suite.doTest1();
    }
}
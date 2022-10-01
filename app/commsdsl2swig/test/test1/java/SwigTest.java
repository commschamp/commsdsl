public class SwigTest {
    private test1_message_Msg1 m_msg1 = null;
    private test1_message_Msg2 m_msg2 = null;

    public void setMsg1(test1_message_Msg1 msg) {
        m_msg1 = msg;
    }

    public void setMsg2(test1_message_Msg2 msg) {
        m_msg2 = msg;
    }

    static {
        System.loadLibrary("commsdsl2swig_test1_java");
    }

    public void doTest1() {
        var msg = new test1_message_Msg1();
        msg.field_f1().setValue(0x123456);
        msg.field_f2().setValue(300);

        var frame = new test1_frame_Frame();
        var buf = frame.writeMessage(msg);
        var handler = new TestMsgHandler(this);

        var consumed = frame.processInputData(buf, handler);
        assert consumed == buf.size();
        assert test1.eq_test1_message_Msg1(msg, m_msg1): "Messages not equal";
        System.out.println("Test1 Complete");
    }

    public static void main(String argv[]) {
        var suite = new SwigTest();
        suite.doTest1();
    }
}
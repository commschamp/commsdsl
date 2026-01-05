public class SwigTest {
    private t20_swig.message_Msg1 m_msg1 = null;
    private t20_swig.message_Msg2 m_msg2 = null;

    static {
        System.loadLibrary("t20_swig_java");
    }

    public void setMsg1(t20_swig.message_Msg1 msg) {
        m_msg1 = msg;
    }

    public void setMsg2(t20_swig.message_Msg2 msg) {
        m_msg2 = msg;
    }

    public void doTest1() {
        var msg = new t20_swig.message_Msg2();
        msg.field_f1().ref().setMeters(0.1);
        assert msg.field_f1().ref().getMeters() == 0.1;
        assert msg.field_f1().ref().getScaled() == 100.0;
        assert msg.field_f1().ref().getValue() == 10000;

        var frame = new t20_swig.frame_Frame();
        var buf = frame.writeMessage(msg);
        var handler = new TestMsgHandler(this);

        var consumed = frame.processInputData(buf, handler);
        assert consumed == buf.size();
        assert t20_swig.t20.eq_message_Msg2(msg, m_msg2): "Messages not equal";
        System.out.println("Test1 Complete");
    }

    public static void main(String argv[]) {
        var suite = new SwigTest();
        suite.doTest1();
    }
}
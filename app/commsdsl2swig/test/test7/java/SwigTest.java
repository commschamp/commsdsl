public class SwigTest {
    private test7_message_Msg1 m_msg1 = null;

    static {
        System.loadLibrary("commsdsl2swig_test7_java");
    }

    public void setMsg1(test7_message_Msg1 msg) {
        m_msg1 = msg;
    }    

    public void doTest1() {
        var msg = new test7_message_Msg1();
        assert msg.field_f1().ref().getBitValue_b2();
        assert !msg.field_f1().ref().getBitValue_b3();
        msg.field_f1().ref().setValue(0xff);
        msg.field_f2().setBitValue_b3(true);
        assert msg.field_f2().getValue() == 0x8;

        var frame = new test7_frame_Frame();
        var buf = frame.writeMessage(msg);
        var handler = new TestMsgHandler(this);

        var consumed = frame.processInputData(buf, handler);
        assert consumed == buf.size();
        assert test7.eq_test7_message_Msg1(msg, m_msg1): "Messages not equal";
        System.out.println("Test1 Complete");
    }

    public static void main(String argv[]) {
        var suite = new SwigTest();
        suite.doTest1();
    }
}
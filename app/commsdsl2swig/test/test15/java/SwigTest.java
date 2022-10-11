public class SwigTest {
    private test15_swig.message_Msg1 m_msg1 = null;

    static {
        System.loadLibrary("test15_swig_java");
    }

    public void setMsg1(test15_swig.message_Msg1 msg) {
        m_msg1 = msg;
    }    

    public void doTest1() {
        var msg = new test15_swig.message_Msg1();
        assert msg.field_f10().doesExist();

        msg.field_f1().ref().setValue(new test15_swig.DataBuf(new short[]{0x01, 0x02, 0x03, 0x00, 0x00}));
        msg.field_f2().ref().setValue(new test15_swig.DataBuf(new short[]{0x04, 0x05, 0x06}));
        msg.field_f3().ref().setValue(new test15_swig.DataBuf(new short[]{0x07, 0x08}));
        msg.field_f10().field().setValue(new test15_swig.DataBuf(new short[]{0x0a, 0x0b}));

        var frame = new test15_swig.frame_Frame();
        var buf = frame.writeMessage(msg);
        var handler = new TestMsgHandler(this);

        var consumed = frame.processInputData(buf, handler);
        assert consumed == buf.size();
        assert test15_swig.test15.eq_message_Msg1(msg, m_msg1): "Messages not equal";
        System.out.println("Test1 Complete");
    }

    public static void main(String argv[]) {
        var suite = new SwigTest();
        suite.doTest1();
    }
}
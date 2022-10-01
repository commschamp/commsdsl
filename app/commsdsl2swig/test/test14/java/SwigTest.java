public class SwigTest {
    private test14_message_Msg1 m_msg1 = null;

    static {
        System.loadLibrary("commsdsl2swig_test14_java");
    }

    public void setMsg1(test14_message_Msg1 msg) {
        m_msg1 = msg;
    }    

    public void doTest1() {
        var msg = new test14_message_Msg1();
        System.out.println("f1=" + msg.field_f1().ref().getValue());

        assert msg.field_f1().ref().getValue().equals("hello");
        assert msg.field_f2().ref().getValue().equals("hello");
        assert msg.field_f3().ref().getValue().equals("hello");
        assert msg.field_f4().ref().getValue().isEmpty();
        assert msg.field_f10().getValue().isEmpty();

        msg.field_f1().ref().setValue("bla");
        msg.field_f2().ref().setValue("aaa");
        msg.field_f3().ref().setValue("bbb");
        msg.field_f4().ref().setValue("ccc");
        msg.field_f10().setValue("ddd");        

        var frame = new test14_frame_Frame();
        var buf = frame.writeMessage(msg);
        var handler = new TestMsgHandler(this);

        var consumed = frame.processInputData(buf, handler);
        assert consumed == buf.size();
        assert test14.eq_test14_message_Msg1(msg, m_msg1): "Messages not equal";
        System.out.println("Test1 Complete");
    }

    public static void main(String argv[]) {
        var suite = new SwigTest();
        suite.doTest1();
    }
}
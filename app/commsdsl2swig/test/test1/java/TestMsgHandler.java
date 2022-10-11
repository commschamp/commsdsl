public class TestMsgHandler extends test1_swig.MsgHandler {
    private SwigTest m_test;

    public TestMsgHandler(SwigTest test) {
        m_test = test;
    }

    public void handle_message_Msg1(test1_swig.message_Msg1 msg) {
        m_test.setMsg1(new test1_swig.message_Msg1(msg));
    }

    public void handle_message_Msg2(test1_swig.message_Msg2 msg) {
        m_test.setMsg2(new test1_swig.message_Msg2(msg));
    }

    public void handle_Message(test1_swig.Message msg) {
        assert false;
    }
}
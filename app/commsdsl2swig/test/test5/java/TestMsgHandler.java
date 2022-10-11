public class TestMsgHandler extends test5_swig.MsgHandler {
    private SwigTest m_test;

    public TestMsgHandler(SwigTest test) {
        m_test = test;
    }

    public void handle_message_Msg1(test5_swig.message_Msg1 msg) {
        m_test.setMsg1(new test5_swig.message_Msg1(msg));
    }

    public void handle_Message(test5_swig.Message msg) {
        assert false;
    }
}
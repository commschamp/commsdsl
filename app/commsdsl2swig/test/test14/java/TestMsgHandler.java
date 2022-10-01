public class TestMsgHandler extends test14_MsgHandler {
    private SwigTest m_test;

    public TestMsgHandler(SwigTest test) {
        m_test = test;
    }

    public void handle_test14_message_Msg1(test14_message_Msg1 msg) {
        m_test.setMsg1(new test14_message_Msg1(msg));
    }

    public void handle_test_Message(test14_Message msg) {
        assert false;
    }
}
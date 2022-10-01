public class TestMsgHandler extends test5_MsgHandler {
    private SwigTest m_test;

    public TestMsgHandler(SwigTest test) {
        m_test = test;
    }

    public void handle_test5_message_Msg1(test5_message_Msg1 msg) {
        m_test.setMsg1(new test5_message_Msg1(msg));
    }

    public void handle_test_Message(test5_Message msg) {
        assert false;
    }
}
public class TestMsgHandler extends test7_MsgHandler {
    private SwigTest m_test;

    public TestMsgHandler(SwigTest test) {
        m_test = test;
    }

    public void handle_test7_message_Msg1(test7_message_Msg1 msg) {
        m_test.setMsg1(new test7_message_Msg1(msg));
    }

    public void handle_test_Message(test7_Message msg) {
        assert false;
    }
}
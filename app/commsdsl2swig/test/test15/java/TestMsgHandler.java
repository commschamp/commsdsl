public class TestMsgHandler extends test15_MsgHandler {
    private SwigTest m_test;

    public TestMsgHandler(SwigTest test) {
        m_test = test;
    }

    public void handle_test15_message_Msg1(test15_message_Msg1 msg) {
        m_test.setMsg1(new test15_message_Msg1(msg));
    }

    public void handle_test_Message(test15_Message msg) {
        assert false;
    }
}
public class TestMsgHandler extends test1_MsgHandler {
    private SwigTest m_test;

    public TestMsgHandler(SwigTest test) {
        m_test = test;
    }

    public void handle_test1_message_Msg1(test1_message_Msg1 msg) {
        m_test.setMsg1(new test1_message_Msg1(msg));
    }

    public void handle_test1_message_Msg2(test1_message_Msg2 msg) {
        m_test.setMsg2(new test1_message_Msg2(msg));
    }

    public void handle_test_Message(test1_Message msg) {
        assert false;
    }
}
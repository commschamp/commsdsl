public class TestMsgHandler extends MsgHandler {
    private SwigTest m_test;

    public TestMsgHandler(SwigTest test) {
        m_test = test;
    }

    public void handle_message_Msg1(message_Msg1 msg) {
        m_test.setMsg1(new message_Msg1(msg));
    }

    public void handle_message_Msg2(message_Msg2 msg) {
        m_test.setMsg2(new message_Msg2(msg));
    }

    public void handle_Message(Message msg) {
        assert false;
    }
}
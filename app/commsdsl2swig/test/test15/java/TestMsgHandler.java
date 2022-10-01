public class TestMsgHandler extends test15_p.MsgHandler {
    private SwigTest m_test;

    public TestMsgHandler(SwigTest test) {
        m_test = test;
    }

    public void handle_message_Msg1(test15_p.message_Msg1 msg) {
        m_test.setMsg1(new test15_p.message_Msg1(msg));
    }

    public void handle_Message(test15_p.Message msg) {
        assert false;
    }
}
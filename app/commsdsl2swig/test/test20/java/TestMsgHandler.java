public class TestMsgHandler extends test20_p.MsgHandler {
    private SwigTest m_test;

    public TestMsgHandler(SwigTest test) {
        m_test = test;
    }

    public void handle_message_Msg1(test20_p.message_Msg1 msg) {
        m_test.setMsg1(new test20_p.message_Msg1(msg));
    }

    public void handle_message_Msg2(test20_p.message_Msg2 msg) {
        m_test.setMsg2(new test20_p.message_Msg2(msg));
    }    

    public void handle_Message(test20_p.Message msg) {
        assert false;
    }
}
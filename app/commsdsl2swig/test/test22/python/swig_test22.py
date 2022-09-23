import os
import sys
import unittest

sys.path.append(os.getcwd())

import test22

class MsgHandler(test22.frame_Frame_Handler):

    def __init__(self, testObj = None):
        test22.frame_Frame_Handler.__init__(self)
        self.testObj = testObj

    def handle_message_Msg1(self, msg):
        self.msg1 = True
        if (self.testObj is not None):
            self.testObj.msg1 = test22.message_Msg1(msg)

    def handle_message_Msg2(self, msg):
        self.msg2 = True
        if (self.testObj is not None):
            self.testObj.msg2 = test22.message_Msg2(msg)    

    def handle_message_Msg3(self, msg):
        self.msg3 = True
        if (self.testObj is not None):
            self.testObj.msg3 = test22.message_Msg3(msg)                         

    def handle_Message(self, msg):
        sys.exit("shouldn't happen")

class TestProtocol(unittest.TestCase):
    def test_1(self):
        m = test22.message_Msg1()
        self.assertTrue(m.field_f1().field_long().isMissing())
        
        m.field_f1().field_short().setValue(0)
        m.refresh()
        self.assertTrue(m.field_f1().field_long().doesExist())
        m.field_f1().field_long().field().setValue(0x1234)

        frame = test22.frame_Frame()
        buf = frame.writeMessage(m)

        h = MsgHandler(self)
        frame.processInputData(buf, h)

        self.assertTrue(test22.eq_message_Msg1(self.msg1, m))


if __name__ == '__main__':
    unittest.main()



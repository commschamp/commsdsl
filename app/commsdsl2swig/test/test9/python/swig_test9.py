import os
import sys
import unittest
import math

sys.path.append(os.getcwd())

import test9

class MsgHandler(test9.frame_Frame_Handler):

    def __init__(self, testObj = None):
        test9.frame_Frame_Handler.__init__(self)
        self.testObj = testObj

    def handle_message_Msg1(self, msg):
        self.msg1 = True
        if (self.testObj is not None):
            self.testObj.msg1 = test9.message_Msg1(msg)

    def handle_Message(self, msg):
        sys.exit("shouldn't happen")

class TestProtocol(unittest.TestCase):
    def setUp(self): 
        self.msg1 = None

    def test_1(self):
        m = test9.message_Msg1()
        self.assertTrue(math.isnan(m.field_f1().getValue()))
        self.assertTrue(m.field_f1().isNull())
        m.field_f1().setS1()
        self.assertTrue(m.field_f1().isS1())
        self.assertTrue(math.isinf(m.field_f1().getValue()))
        m.field_f1().setS4()

        frame = test9.frame_Frame()
        buf = frame.writeMessage(m)

        h = MsgHandler(self)
        frame.processInputData(buf, h)

        self.assertTrue(h.msg1)
        self.assertTrue(test9.eq_message_Msg1(self.msg1, m))

if __name__ == '__main__':
    unittest.main()



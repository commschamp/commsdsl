import os
import sys
import unittest
import pickle

sys.path.append(os.getcwd())

import test7

class MsgHandler(test7.frame_Frame_Handler):

    def __init__(self, testObj = None):
        test7.frame_Frame_Handler.__init__(self)
        self.testObj = testObj

    def handle_message_Msg1(self, msg):
        self.msg1 = True
        if (self.testObj is not None):
            self.testObj.msg1 = test7.message_Msg1(msg)

    def handle_Message(self, msg):
        sys.exit("shouldn't happen")

class TestProtocol(unittest.TestCase, test7.frame_Frame_Handler):
    def test_1(self):
        m = test7.message_Msg1()
        self.assertEqual(m.field_f1().getBitValue_b2(), True)
        self.assertEqual(m.field_f1().getBitValue_b3(), False)
        m.field_f1().setValue(0xff)
        m.field_f2().setBitValue_b3(True)
        self.assertEqual(m.field_f2().getValue(), 0x8)

        frame = test7.frame_Frame()
        buf = frame.writeMessage(m)

        h = MsgHandler(self)
        frame.processInputData(buf, h)
                
        self.assertEqual(self.msg1.field_f1().getValue(), m.field_f1().getValue())
        self.assertTrue(test7.eq_message_Msg1(self.msg1, m))


if __name__ == '__main__':
    unittest.main()



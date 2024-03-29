import os
import sys
import unittest

import test7

class MsgHandler(test7.MsgHandler):

    def __init__(self, testObj = None):
        test7.MsgHandler.__init__(self)
        self.testObj = testObj

    def handle_message_Msg1(self, msg):
        self.msg1 = True
        if (self.testObj is not None):
            self.testObj.msg1 = test7.message_Msg1(msg)

    def handle_Message(self, msg):
        sys.exit("shouldn't happen")

class TestProtocol(unittest.TestCase):
    def test_1(self):
        m = test7.message_Msg1()
        self.assertEqual(m.field_f1().ref().getBitValue_b2(), True)
        self.assertEqual(m.field_f1().ref().getBitValue_b3(), False)
        m.field_f1().ref().setValue(0xff)
        m.field_f2().setBitValue_b3(True)
        self.assertEqual(m.field_f2().getValue(), 0x8)

        frame = test7.frame_Frame()
        buf = frame.writeMessage(m)

        h = MsgHandler(self)
        frame.processInputData(buf, h)
                
        self.assertEqual(self.msg1.field_f1().ref().getValue(), m.field_f1().ref().getValue())
        self.assertTrue(test7.eq_message_Msg1(self.msg1, m))

    def test_2(self):
        m = test7.message_Msg1()

        frame = test7.frame_Frame()
        buf = frame.writeMessage(m)

        h = MsgHandler(self)
        frame.processInputData(buf, h)
        frameFields = test7.frame_Frame_AllFields()
        frame.processInputDataSingleMsg(buf, h, frameFields)

        self.assertEqual(self.msg1.field_f1().ref().getValue(), m.field_f1().ref().getValue())
        self.assertEqual(frameFields.m_iD.getValue(), 1)
        self.assertEqual(bytearray(frameFields.m_data.getValue()), bytearray(b'\x00\x00\x06\x00'))

if __name__ == '__main__':
    unittest.main()



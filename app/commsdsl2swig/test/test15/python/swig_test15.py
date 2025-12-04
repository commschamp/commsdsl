import os
import sys
import unittest

import test15

class MsgHandler(test15.MsgHandler):

    def __init__(self, testObj = None):
        test15.MsgHandler.__init__(self)
        self.testObj = testObj

    def handle_message_Msg1(self, msg):
        self.msg1 = True
        if (self.testObj is not None):
            self.testObj.msg1 = test15.message_Msg1(msg)

    def handle_Message(self, msg):
        sys.exit("shouldn't happen")

class TestProtocol(unittest.TestCase):
    def test_1(self):
        m = test15.message_Msg1()
        self.assertEqual(bytearray(m.field_f1().ref().getValue()), bytearray(b'\x0a\x0b\x0c\x0d\x01'))
        self.assertTrue(m.field_f10().doesExist())

        m.field_f1().ref().setValue(bytearray(b'\x01\x02\x03\x00\x00'));
        m.field_f2().ref().setValue(bytearray(b'\x04\x05\x06'));
        m.field_f3().ref().setValue(bytearray(b'\x07\x08'));
        m.field_f10().field().setValue(bytearray(b'\x0a\x0b'));

        frame = test15.frame_Frame()
        buf = frame.writeMessage(m)

        h = MsgHandler(self)
        frame.processInputData(buf, h)

        self.assertTrue(self.msg1.field_f10().doesExist())
        self.assertTrue(test15.eq_message_Msg1(self.msg1, m))

    def test_2(self):
        m = test15.message_Msg1()
        m.transportField_version().setValue(1)
        m.refresh()
        self.assertTrue(m.field_f10().isMissing())

        frame = test15.frame_Frame()
        buf = frame.writeMessage(m)

        self.msg1 = None
        h = MsgHandler(self)
        frame.processInputData(buf, h)

        # Due to missing version handling in the frame
        # the message can't be created
        self.assertIsNone(self.msg1)

if __name__ == '__main__':
    unittest.main()


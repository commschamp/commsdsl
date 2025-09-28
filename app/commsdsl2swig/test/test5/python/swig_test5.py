import os
import sys
import unittest

import test5

class MsgHandler(test5.MsgHandler):

    def __init__(self, msgFunc = None):
        test5.MsgHandler.__init__(self)
        self.msgFunc = msgFunc

    def handle_message_Msg1(self, msg):
        self.msg1 = True
        if (self.msgFunc is not None):
            self.msgFunc(msg)

    def handle_Message(self, msg):
        sys.exit("shouldn't happen")

class TestProtocol(unittest.TestCase):
    def test_1(self):
        f1 = 0
        f2 = 0
        f3 = 0
        f4 = 0
        def test_msg1(msg):
            nonlocal f1
            nonlocal f2
            nonlocal f3
            nonlocal f4
            f1 = msg.field_f1().ref().getValue()
            f2 = msg.field_f2().ref().getValue()
            f3 = msg.field_f3().ref().getValue()
            f4 = msg.field_f4().getValue()

        f = test5.frame_Frame()
        h = MsgHandler(test_msg1)
        buf = bytearray(b'\x01\x00\x02\x05\x00\x0a\x03')
        f.processInputData(buf, h)
        self.assertEqual(h.msg1, True)
        #self.assertEqual(f1, test5.field_E1.ValueType_V2)
        self.assertEqual(f1, test5.field_E1.ValueType_V2)
        self.assertEqual(f2, test5.field_MsgId.ValueType_m5)
        self.assertEqual(f3, test5.field_E2.ValueType_V10)
        self.assertEqual(f4, test5.message_Msg1Fields_F4.ValueType_V3)

        m = test5.message_Msg1()
        m.field_f1().ref().setValue(f1)
        m.field_f2().ref().setValue(f2)
        m.field_f3().ref().setValue(f3)
        m.field_f4().setValue(f4)
        outBuf = f.writeMessage(m)

        self.assertEqual(bytearray(outBuf), buf)
        self.assertEqual(m.field_f4().valueName(), test5.message_Msg1Fields_F4.valueNameOf(test5.message_Msg1Fields_F4.ValueType_V3))

if __name__ == '__main__':
    unittest.main()


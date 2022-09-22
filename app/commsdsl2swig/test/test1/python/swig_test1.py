import os
import sys
import unittest

sys.path.append(os.getcwd())

import test1

class MsgHandler(test1.frame_Frame_Handler):

    def __init__(self, msgFunc = None):
        test1.frame_Frame_Handler.__init__(self)
        self.msgFunc = msgFunc

    def handle_message_Msg1(self, msg):
        self.msg1 = True
        if (self.msgFunc is not None):
            self.msgFunc(msg)

    def handle_message_Msg2(self, msg):
        self.msg2 = True
        if (self.msgFunc is not None):
            self.msgFunc(msg)

    def handle_Message(self, msg):
        sys.exit("shouldn't happen")


class TestProtocol(unittest.TestCase):
    def test_1(self):
        f1 = 0
        f2 = 0
        def test_msg1(msg):
            nonlocal f1
            nonlocal f2
            f1 = msg.field_f1().getValue()
            f2 = msg.field_f2().getValue()

        f = test1.frame_Frame()
        h = MsgHandler(test_msg1)
        buf = bytearray(b'\x01\x01\x02\x03\x04')
        f.processInputData(buf, h)
        self.assertEqual(h.msg1, True)
        self.assertEqual(f1, 0x030201)
        self.assertEqual(f2, 0x04)

    def test_2(self):
        f = test1.frame_Frame()
        h = MsgHandler()
        buf = bytearray(b'\x02')
        f.processInputData(buf, h)
        self.assertEqual(h.msg2, True)

    def test_3(self):
        m = test1.message_Msg1();
        m.field_f1().setValue(0x010101)
        m.field_f2().setValue(0x02)
        f = test1.frame_Frame()
        outData = f.writeMessage(m)
        self.assertEqual(bytearray(outData), bytearray(b'\x01\x01\x01\x01\x02'))


if __name__ == '__main__':
    unittest.main()



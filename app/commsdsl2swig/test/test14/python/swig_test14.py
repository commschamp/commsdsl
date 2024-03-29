import os
import sys
import unittest

import test14

class MsgHandler(test14.MsgHandler):

    def __init__(self, testObj = None):
        test14.MsgHandler.__init__(self)
        self.testObj = testObj

    def handle_message_Msg1(self, msg):
        self.msg1 = True
        if (self.testObj is not None):
            self.testObj.msg1 = test14.message_Msg1(msg)

    def handle_Message(self, msg):
        sys.exit("shouldn't happen")

class TestProtocol(unittest.TestCase):
    def test_1(self):
        m = test14.message_Msg1()
        self.assertEqual(m.field_f1().ref().getValue(), "hello")
        self.assertEqual(m.field_f2().ref().getValue(), "hello")
        self.assertEqual(m.field_f3().ref().getValue(), "hello")
        self.assertEqual(m.field_f4().ref().getValue(), "")
        self.assertEqual(m.field_f10().getValue(), "")

        m.field_f1().ref().setValue("bla");
        m.field_f2().ref().setValue("aaa");
        m.field_f3().ref().setValue("bbb");
        m.field_f4().ref().setValue("ccc");
        m.field_f10().setValue("ddd");

        frame = test14.frame_Frame()
        buf = frame.writeMessage(m)

        h = MsgHandler(self)
        frame.processInputData(buf, h)
                
        self.assertTrue(test14.eq_message_Msg1(self.msg1, m))
        self.assertEqual(self.msg1.field_f1().ref().getValue(), "bla")


if __name__ == '__main__':
    unittest.main()



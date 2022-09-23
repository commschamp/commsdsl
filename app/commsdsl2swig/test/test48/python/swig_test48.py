import os
import sys
import unittest

sys.path.append(os.getcwd())

import test48

class MsgHandler(test48.frame_Frame_Handler):

    def __init__(self, testObj = None):
        test48.frame_Frame_Handler.__init__(self)
        self.testObj = testObj

    def handle_message_Msg1(self, msg):
        self.msg1 = True
        if (self.testObj is not None):
            self.testObj.msg1 = test48.message_Msg1(msg)

    def handle_message_Msg2(self, msg):
        self.msg2 = True
        if (self.testObj is not None):
            self.testObj.msg2 = test48.message_Msg2(msg) 

    def handle_message_Msg3(self, msg):
        self.msg3 = True
        if (self.testObj is not None):
            self.testObj.msg3 = test48.message_Msg3(msg)

    def handle_message_Msg4(self, msg):
        self.msg4 = True
        if (self.testObj is not None):
            self.testObj.msg4 = test48.message_Msg4(msg)

    def handle_message_Msg5(self, msg):
        self.msg5 = True
        if (self.testObj is not None):
            self.testObj.msg5 = test48.message_Msg5(msg)

    def handle_message_Msg6(self, msg):
        self.msg6 = True
        if (self.testObj is not None):
            self.testObj.msg6 = test48.message_Msg6(msg)

    def handle_Message(self, msg):
        sys.exit("shouldn't happen")

class TestProtocol(unittest.TestCase):
    def test_1(self):
        f = test48.field_Length()
        f.setValue(123)
        self.assertEqual(f.getValue(), 123);
        self.assertEqual(f.field_short().getValue(), 123);
        self.assertTrue(f.field_long().isMissing());

        f.setValue(300)
        self.assertEqual(f.getValue(), 300);
        self.assertEqual(f.field_short().getValue(), 0xff);
        self.assertTrue(f.field_long().doesExist());
        self.assertEqual(f.field_long().field().getValue(), 300);

    def test_2(self):
        f = test48.field_Length2()
        f.setValue(123)
        self.assertEqual(f.getValue(), 123);
        self.assertEqual(f.field_len().getValue(), 123);


if __name__ == '__main__':
    unittest.main()



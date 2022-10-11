import os
import sys
import unittest

import test23

class MsgHandler(test23.MsgHandler):

    def __init__(self, testObj = None):
        test23.MsgHandler.__init__(self)
        self.testObj = testObj

    def handle_message_Msg1(self, msg):
        self.msg1 = True
        if (self.testObj is not None):
            self.testObj.msg1 = test23.message_Msg1(msg)

    def handle_message_Msg2(self, msg):
        self.msg2 = True
        if (self.testObj is not None):
            self.testObj.msg2 = test23.message_Msg2(msg)    

    def handle_message_Msg3(self, msg):
        self.msg3 = True
        if (self.testObj is not None):
            self.testObj.msg3 = test23.message_Msg3(msg)     

    def handle_message_Msg4(self, msg):
        self.msg4 = True
        if (self.testObj is not None):
            self.testObj.msg4 = test23.message_Msg4(msg)                        

    def handle_Message(self, msg):
        sys.exit("shouldn't happen")

class TestProtocol(unittest.TestCase):
    def test_1(self):
        m = test23.message_Msg4()
        self.assertTrue(m.field_b1().ref().field_data2().doesExist())
        
        frame = test23.frame_Frame()
        buf = frame.writeMessage(m)

        h = MsgHandler(self)
        frame.processInputData(buf, h)

        self.assertTrue(test23.eq_message_Msg4(self.msg4, m))

    def test_2(self):
        m = test23.message_Msg4()
        self.assertTrue(m.field_b1().ref().field_data2().doesExist())
        m.transportField_version().setValue(1)
        m.refresh()
        self.assertTrue(m.field_b1().ref().field_data2().isMissing())
        
        frame = test23.frame_Frame()
        buf = frame.writeMessage(m)

        h = MsgHandler(self)
        frame.processInputData(buf, h)

        self.assertTrue(test23.eq_message_Msg4(self.msg4, m))        


if __name__ == '__main__':
    unittest.main()



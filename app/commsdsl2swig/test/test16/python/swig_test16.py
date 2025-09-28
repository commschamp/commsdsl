import os
import sys
import unittest

import test16

class MsgHandler(test16.MsgHandler):

    def __init__(self, testObj = None):
        test16.MsgHandler.__init__(self)
        self.testObj = testObj

    def handle_message_Msg1(self, msg):
        self.msg1 = True
        if (self.testObj is not None):
            self.testObj.msg1 = test16.message_Msg1(msg)

    def handle_message_Msg2(self, msg):
        self.msg2 = True
        if (self.testObj is not None):
            self.testObj.msg2 = test16.message_Msg2(msg)

    def handle_Message(self, msg):
        sys.exit("shouldn't happen")

class TestProtocol(unittest.TestCase):
    def test_1(self):
        m = test16.message_Msg1()

        f1Vec = m.field_f1().ref().value()
        f1Vec.resize(5)
        f1Vec[0].setValue(0x1111)
        f1Vec[1].setValue(0x2222)
        # The rest are defaulted

        f2Vec = m.field_f2().ref().value()
        f2Vec.resize(3)
        f2Vec[0].setValue(0x3333)
        f2Vec[1].setValue(0x4444)
        f2Vec[2].setValue(0x5555)

        f3Vec = m.field_f3().ref().value()
        f3Vec.resize(2)
        f3Vec[0].setValue(0x6666)
        f3Vec[1].setValue(0x7777)

        # Due to version the F4 is Optional wrapping the vector
        self.assertTrue(m.field_f4().doesExist())
        f4Vec = m.field_f4().field().value()
        f4Vec.resize(2)
        f4Vec[0].setValue(0x8888)
        f4Vec[1].setValue(0x9999)

        m.refresh() # Just in case

        frame = test16.frame_Frame()
        buf = frame.writeMessage(m)

        print("Output buffer:", buf)

        h = MsgHandler(self)
        frame.processInputData(buf, h)

        self.assertTrue(h.msg1)
        self.assertTrue(test16.eq_message_Msg1(self.msg1, m))

if __name__ == '__main__':
    unittest.main()


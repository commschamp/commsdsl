import os
import sys
import unittest

sys.path.append(os.getcwd())

import test28

class MsgHandler(test28.MsgHandler):

    def __init__(self, testObj = None):
        test28.MsgHandler.__init__(self)
        self.testObj = testObj

    def handle_message_Msg1(self, msg):
        self.msg1 = True
        if (self.testObj is not None):
            self.testObj.msg1 = test28.message_Msg1(msg)

    def handle_message_Msg2(self, msg):
        self.msg2 = True
        if (self.testObj is not None):
            self.testObj.msg2 = test28.message_Msg2(msg)    

    def handle_Message(self, msg):
        sys.exit("shouldn't happen")

class Variant1Handler(test28.field_Variant1_Handler):

    def __init__(self, testObj = None):
        test28.field_Variant1_Handler.__init__(self)
        self.testObj = testObj

    def handle_p1(self, field):
        self.p1 = True
        if (self.testObj is not None):
            self.testObj.p1 = test28.field_Variant1Members_P1(field)

    def handle_p2(self, field):
        self.p2 = True
        if (self.testObj is not None):
            self.testObj.p2 = test28.field_Variant1Members_P2(field)    

    def handle_Message(self, msg):
        sys.exit("shouldn't happen")        

class TestProtocol(unittest.TestCase):
    def test_1(self):
        m = test28.message_Msg1()
        self.assertFalse(m.field_variant1().valid())
        self.assertEqual(m.field_variant1().currentField(), 2)
        m.field_variant1().initField_p2()
        self.assertEqual(m.field_variant1().currentField(), 1)
        self.assertEqual(m.field_variant2().currentField(), 0)


        frame = test28.frame_Frame()
        buf = frame.writeMessage(m)

        h = MsgHandler(self)
        frame.processInputData(buf, h)

        self.assertTrue(test28.eq_message_Msg1(self.msg1, m))

        vh = Variant1Handler(self)
        self.msg1.field_variant1().currentFieldExec(vh);   
        self.assertTrue(vh.p2)
        self.assertTrue(test28.eq_field_Variant1Members_P2(self.p2, m.field_variant1().accessField_p2()))             

        m.field_variant1().reset()


if __name__ == '__main__':
    unittest.main()



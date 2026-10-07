import os
import sys
import unittest

import test11

class TestProtocol(unittest.TestCase):
    def test_1(self):
        m = test11.message_Msg11()
        m.field_f1().setBits(test11.message_Msg11Fields_F1.BitMask_M1)
        self.assertTrue(m.field_f1().getBitValue_B0())
        self.assertTrue(m.field_f1().getBitValue_B1())

if __name__ == '__main__':
    unittest.main()


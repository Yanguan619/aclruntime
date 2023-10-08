import unittest
import numpy as np
from ptdbg_ascend.parse_tool.lib.visualization import Visualization

class TestVisualization(unittest.TestCase):
    def setUp(self):
        self.visualization = Visualization()

    def test_print_npy_summary(self):
        np.save('test.npy', np.array([1, 2, 3, 4, 5]))
        try:
            self.visualization.print_npy_summary('test.npy')
        except Exception as e:
            self.fail(f"print_npy_summary raised exception {e}")

    def test_print_npy_data(self):
        np.save('test.npy', np.array([1, 2, 3, 4, 5]))
        try:
            self.visualization.print_npy_data('test.npy')
        except Exception as e:
            self.fail(f"print_npy_data raised exception {e}")

    def test_parse_pkl(self):
        with open('test.pkl', 'w') as f:
            f.write('{"0": "test", "1": [], "2": "", "3": "float32", "4": [1, 2], "5": [0, 1, 0.5]}')
        try:
            self.visualization.parse_pkl('test.pkl', 'test')
        except Exception as e:
            self.fail(f"parse_pkl raised exception {e}")


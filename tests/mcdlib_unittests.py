import unittest
import time

from internals.mcdlib_routines import RoutinesMcdRefSrv

class TestMcdRefSrv(unittest.TestCase):
    routines = RoutinesMcdRefSrv()

    def setUp(self):
        self.assertFalse(self.routines.setup_mcdrefsrv())
        self.assertFalse(self.routines.start_routine())
        self.assertFalse(self.routines.target_query_routine())
        self.assertFalse(self.routines.open_cores_routine())
        self.assertFalse(self.routines.core_query_routine())
        self.assertFalse(self.routines.break_init_routine())

    def test_go_break_pos(self):
        self.assertFalse(self.routines.state_routine())
        self.assertFalse(self.routines.read_pc_routine())
        self.assertFalse(self.routines.go_routine())
        time.sleep(0.1)
        self.assertFalse(self.routines.break_routine())
        self.assertFalse(self.routines.state_routine())
        self.assertFalse(self.routines.read_pc_routine())

    def test_step(self):
        self.assertFalse(self.routines.state_routine())
        self.assertFalse(self.routines.read_pc_routine())
        self.assertFalse(self.routines.step_routine())
        self.assertFalse(self.routines.read_pc_routine())
        self.assertFalse(self.routines.step_routine())
        self.assertFalse(self.routines.read_pc_routine())

    def tearDown(self):
        self.assertFalse(self.routines.close_cores_routine())
        self.assertFalse(self.routines.shutdown_routine())

if __name__ == '__main__':
    unittest.main()

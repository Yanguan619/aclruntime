import os

import unittest
from unittest.mock import patch


from ais_bench.net_test.security.file_checker import check_linux_file_path
from ais_bench.net_test.security.standard_consts import PermNeed, PermForbid, FileSizeLimit
from tests.utils import FileCreationForTesting


class TestCheckLinuxFilePath(unittest.TestCase):
    def setUp(self):
        self.file_creator = FileCreationForTesting()

    def tearDown(self):
        self.file_creator.clean_up()

    def test_existing_file(self):
        file_path = self.file_creator.create_file('test.txt', permission=0o640)
        check_linux_file_path(str(file_path))

    def test_non_existent_file(self):
        with self.assertRaisesRegex(ValueError, 'path not exist.') as context:
            check_linux_file_path(str(self.file_creator.base_path / 'non_existent.txt'))

    def test_directory_not_file(self):
        dir_path = self.file_creator.base_path / 'test_dir'
        os.mkdir(dir_path)
        with self.assertRaisesRegex(ValueError, 'path is not a file.'):
            check_linux_file_path(str(dir_path))

    def test_softlink(self):
        file_path = self.file_creator.create_file('test.txt')
        softlink_path = os.path.join(self.file_creator.base_path, 'test_link.txt')
        os.symlink(file_path, softlink_path)
        with self.assertRaisesRegex(ValueError, 'path is a softlink.'):
            check_linux_file_path(str(softlink_path))

        softlink_path = self.file_creator.create_file('test2.txt', is_softlink=True)
        with self.assertRaisesRegex(ValueError, 'path is a softlink.'):
            check_linux_file_path(str(softlink_path))

    @patch('os.getgroups')
    @patch('os.geteuid')
    def test_not_user_owner(self, mock_geteuid, mock_getgroups):
        mock_geteuid.return_value = -1
        mock_getgroups.return_value = []

        file_path = self.file_creator.create_file('test.txt',permission=0o640)
        with self.assertRaisesRegex(ValueError, 'path is not belong to current user or user group.'):
            check_linux_file_path(str(file_path))

    def test_illegal_permission(self):
        for permission in [0o641, 0o642, 0o644]:
            file_path = self.file_creator.create_file('test.txt', permission=permission)
            with self.assertRaisesRegex(ValueError, "path's permission is illegal."):
                check_linux_file_path(str(file_path), perm_need=PermNeed.READ_FILE,
                                      perm_forbid=PermForbid.USER_MAIN_DIR)

    def test_size_exceeded(self):
        file_path = self.file_creator.create_file('test.txt', size=11 * 1024 * 1024, permission=0o640)
        with self.assertRaisesRegex(ValueError, 'file size over max size: '):
            check_linux_file_path(str(file_path), max_size=FileSizeLimit.NORMAL_CONFIG_FILE)

    def test_illegal_suffix(self):
        file_path = self.file_creator.create_file('test.unsupported', permission=0o640)
        with self.assertRaisesRegex(ValueError, 'file suffix is not in : '):
            check_linux_file_path(str(file_path), legal_suffixes=['.txt'])


if __name__ == '__main__':
    unittest.main()

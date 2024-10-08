import stat
import unittest
from unittest.mock import patch, MagicMock

from ais_bench.net_test.security.file_stat import FileStat, OpenException


class TestFileStat(unittest.TestCase):
    @patch('os.path.exists', return_value=True)
    @patch('os.stat')
    def test_file_stat_exists(self, mock_stat, mock_exists):
        # Create mock for os.stat result
        mock_stat_result = MagicMock()
        mock_stat_result.st_mode = stat.S_IFREG  # Regular file
        mock_stat_result.st_size = 1024
        mock_stat_result.st_uid = 1000
        mock_stat_result.st_gid = 1000
        mock_stat.return_value = mock_stat_result

        # Test file
        file_stat = FileStat("/fake/path/file.txt")

        assert file_stat.is_exists is True
        assert file_stat.is_file is True
        assert file_stat.is_dir is False
        assert file_stat.file_size == 1024
        assert file_stat.owner == 1000
        assert file_stat.group_owner == 1000
        assert file_stat.suffix == ".txt"

    @patch('os.path.exists', return_value=False)
    def test_file_stat_not_exists(self, mock_exists):
        # Test file that does not exist
        file_stat = FileStat("/fake/path/nonexistent_file.txt")

        assert file_stat.is_exists is False
        assert file_stat.is_file is False
        assert file_stat.is_dir is False
        assert file_stat.file_size == 0
        assert file_stat.owner == -1
        assert file_stat.group_owner == -1
        assert file_stat.suffix == ""

    @patch('os.path.exists', return_value=True)
    @patch('os.stat')
    def test_file_stat_is_directory(self, mock_stat, mock_exists):
        # Create mock for os.stat result for directory
        mock_stat_result = MagicMock()
        mock_stat_result.st_mode = stat.S_IFDIR  # Directory
        mock_stat_result.st_size = 4096
        mock_stat.return_value = mock_stat_result

        # Test directory
        file_stat = FileStat("/fake/path/directory")

        assert file_stat.is_exists is True
        assert file_stat.is_file is False
        assert file_stat.is_dir is True

    @patch('os.path.realpath', return_value='/fake/path/realpath')
    @patch('os.path.exists', return_value=True)
    @patch('os.path.islink', return_value=True)
    @patch('os.stat')
    def test_file_stat_is_symlink(self, mock_stat, mock_islink, mock_exists, mock_realpath):
        # Create mock for os.stat result for symbolic link
        mock_stat_result = MagicMock()
        mock_stat_result.st_mode = stat.S_IFLNK  # Symlink
        mock_stat.return_value = mock_stat_result

        # Test symlink
        file_stat = FileStat("/fake/path/symlink")

        assert file_stat.is_softlink is True
        assert file_stat.is_file is False
        assert file_stat.is_dir is False

    @patch('os.path.exists', return_value=True)
    @patch('os.stat')
    @patch('os.geteuid', return_value=1000)
    @patch('os.getgroups', return_value=[1000])
    def test_file_stat_permissions(self, mock_getgroups, mock_geteuid, mock_stat, mock_exists):
        # Create mock for os.stat result
        mock_stat_result = MagicMock()
        mock_stat_result.st_mode = stat.S_IFREG  # Regular file
        mock_stat_result.st_uid = 1000
        mock_stat_result.st_gid = 1000
        mock_stat.return_value = mock_stat_result

        # Test file permissions and ownership
        file_stat = FileStat("/fake/path/file.txt")

        assert file_stat.is_owner is True
        assert file_stat.is_group_owner is True
        assert file_stat.permission == stat.S_IMODE(mock_stat_result.st_mode)

    def test_file(self):
        with self.assertRaises(OpenException):
            file_stat = FileStat("/fake/path/:/file.txt")

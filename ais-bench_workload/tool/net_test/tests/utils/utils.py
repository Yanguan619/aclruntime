import os
import tempfile
from pathlib import Path


def get_temp_file_dir(using_temp_dir=False):
    if using_temp_dir:
        temp_dir = tempfile.TemporaryDirectory()
        return temp_dir, Path(temp_dir.name)
    else:
        # used for debug
        p = '/tmp/tmp_for_test'
        if not os.path.exists(p):
            os.mkdir(p)
        return None, Path(p)


class FileCreationForTesting:
    def __init__(self):
        if os.environ.get('DEBUG_NET_TEST', ''):
            using_temp_dir = False
        else:
            using_temp_dir = True

        self.temp_dir, self.base_path = get_temp_file_dir(using_temp_dir=using_temp_dir)
        self.using_temp_dir = using_temp_dir

        print('temp file dir base_path:', self.base_path)

    def create_file(self, file_name, content='',
                    size=1024, permission=0o642, is_softlink=False):
        original_file_path = self.base_path / file_name
        with open(original_file_path, 'w') as f:
            f.write(content)
            f.write('\n' * max(size - len(content), 0))
        os.chmod(original_file_path, permission)
        if is_softlink:
            # 获取文件名和扩展名
            file_name_parts = os.path.splitext(file_name)
            new_file_name = f"{file_name_parts[0]}_link{file_name_parts[1]}"
            softlink_path = self.base_path / new_file_name
            os.symlink(original_file_path, softlink_path)
            # 更新 file_path 为软链接路径
            original_file_path = softlink_path
        return original_file_path

    def clean_up(self):
        if self.using_temp_dir:
            self.temp_dir.cleanup()


if __name__ == '__main__':
    file_creator = FileCreationForTesting()
    file = file_creator.create_file('test_in_main.txt', content='')

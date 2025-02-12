import os
import json
import zipfile
import hashlib
import urllib.request

from .logging import get_logger

DATASETS_URL_CONFIG_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../../../ais_bench/datasets/datasets_urls.json")


class DataSetDownLoader:
    def __init__(self, dataset_path):
        self.dataset_path = os.path.abspath(dataset_path)
        with open(DATASETS_URL_CONFIG_PATH, "r") as file:
            self.urls_data = json.load(file)

    def __call__(self):
        self.auto_download_dataset()

    def get_dataset_info_from_path(self):
        path_list = self.dataset_path.split("/") # only linux
        for idx, basename in enumerate(reversed(path_list)):
            if self.urls_data.get(basename):
                save_path = "/" + "/".join(path_list[:idx - 1])
                # need to check save_path
                return save_path, basename
        raise ValueError(f"auto download datasets path is illegal!")

    def calculate_sha256(self, file_path):
        sha256_hash = hashlib.sha256()
        try:
            with open(file_path, 'rb') as file:
                for chunk in iter(lambda: file.read(4096), b""):
                    sha256_hash.update(chunk)
            return sha256_hash.hexdigest()
        except FileNotFoundError:
            return None

    def verify_sha256(self, file_path, expected_sha256):
        actual_sha256 = self.calculate_sha256(file_path)
        if actual_sha256 is not None:
            if actual_sha256 == expected_sha256:
                return
            else:
                raise ValueError("Sha-256 hash info check failed!")

    def auto_download_dataset(self):
        # customized dataset path
        if os.path.exists(self.dataset_path):
            return

        save_path, dataset_name = self.get_dataset_info_from_path()
        dataset_dir_path = os.path.join(save_path, dataset_name)

        if not os.path.exists(save_path):
            os.makedirs(save_path) # need to replace by a safe func

        if os.path.exists(dataset_dir_path):
            get_logger().warning(f"Dataset: {dataset_name} is exist, won't auto download")
            return

        # download dataset zip
        dataset_zip_path = dataset_dir_path + ".zip"
        if not os.path.exists(dataset_zip_path):
            url = self.urls_data.get(dataset_name, None).get("url", None)
            try:
                urllib.request.urlretrieve(url, dataset_zip_path)
            except Exception as err:
                raise RuntimeError(f"auto download dataset: {dataset_name} failed!") from err
            get_logger().info(f"auto download dataset: {dataset_name} success")

        # check hash info
        hash_info = self.urls_data.get(dataset_name, None).get("hash_info", None)
        self.verify_sha256(dataset_zip_path, hash_info)

        with zipfile.ZipFile(dataset_zip_path, 'r') as zip_ref:
            zip_ref.extractall(self.dataset_path)

        os.remove(dataset_zip_path)


def get_cache_dir(default_dir):
    return os.environ.get('AIS_BENCH_DATASETS_CACHE', default_dir)


def get_data_path(dataset_path: str, local_mode: bool = True):
    """return dataset id when getting data from ModelScope/HuggingFace repo, otherwise just
    return local path as is.

    Args:
        dataset_path (str): data path
        local_mode (bool): whether to use local path or
            ModelScope/HuggignFace repo
    """
    # update the path with CACHE_DIR
    default_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../../../") # site-package
    cache_dir = get_cache_dir(default_dir)

    # For absolute path customized by the users, will not auto download dataset
    if dataset_path.startswith('/'):
        return dataset_path

    # For relative path, with CACHE_DIR
    if local_mode:
        local_path = os.path.join(cache_dir, dataset_path)

        # auto download
        downloader = DataSetDownLoader(local_path)
        downloader()

        if not os.path.exists(local_path):
            raise FileExistsError(f'Dataset path: {local_path} is not exist!')
        else:
            return local_path
    else:
        raise FileExistsError('Dataset path is not empty!')
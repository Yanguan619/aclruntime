import os
from .fileio import download_and_extract_archive
from .datasets_info import DATASETS_MAPPING, DATASETS_URL
from .logging import get_logger

USER_HOME = os.path.expanduser("~")
DEFAULT_DATA_FOLDER = os.path.join(USER_HOME, '.cache/opencompass/')


def get_data_path(dataset_id: str, local_mode: bool = False):
    """return dataset id when getting data from ModelScope/HuggingFace repo, otherwise just
    return local path as is.

    Args:
        dataset_id (str): dataset id or data path
        local_mode (bool): whether to use local path or
            ModelScope/HuggignFace repo
    """
    # update the path with CACHE_DIR
    cache_dir = os.environ.get('COMPASS_DATA_CACHE', '')

    # For absolute path customized by the users
    if dataset_id.startswith('/'):
        return dataset_id

    # For relative path, with CACHE_DIR
    if local_mode:
        local_path = os.path.join(cache_dir, dataset_id)
        if not os.path.exists(local_path):
            raise FileExistsError(f'Dataset path: {local_path} is not exist!')
        else:
            return local_path
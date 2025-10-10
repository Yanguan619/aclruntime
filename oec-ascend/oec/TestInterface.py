# encoding: utf-8
from oec.BaseTypes import State


class TestInterface:
    @property
    def name(self) -> str:
        raise NotImplementedError()

    @property
    def tags(self) -> set:
        raise NotImplementedError()
    
    @property
    def group(self):
        raise NotImplementedError()

    def get_origin_path(self) -> str:
        raise NotImplementedError()

    def get_origin_lineno(self) -> int:
        raise NotImplementedError()

    def set_log_dir_path(self, path: str):
        raise NotImplementedError()

    def get_log_dir_path(self) -> str:
        raise NotImplementedError()
    
    @property
    def state(self) -> State:
        raise NotImplementedError()

    def can_continue(self) -> bool:
        raise NotImplementedError()

    def run(self):
        raise NotImplementedError()

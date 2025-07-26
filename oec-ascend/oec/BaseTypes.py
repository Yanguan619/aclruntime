# encoding: utf-8
from enum import Enum, unique


@unique
class State(Enum):
    NOT_RUNNING = "not running"
    RUNNING = "running"

    NOTHING_TO_DO = "nothing to do"
    PASS = "passed"
    WARNING = "warning"
    UNSUPPORTED = "unsupported"
    
    TIMEOUT = "timeout"
    FAIL = "failed"

@unique
class Products(Enum):
    A2 = "A2"
    A3 = "A3"
    A5 = "A5"
    A200 = "A200"
    A300 = "A300"
    A500 = "A500"
    ALL = ["A2", "A3", "A5", "A200", "A300", "A500"]

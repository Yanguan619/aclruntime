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

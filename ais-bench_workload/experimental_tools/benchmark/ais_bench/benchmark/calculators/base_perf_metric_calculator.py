from abc import abstractmethod, ABC

class BasePerfMetricCalculator(ABC):
    def __init__(self, perf_details: dict):
        self.perf_details = perf_details

    @abstractmethod
    def get_common_res(self):
        return {}

    @abstractmethod
    def save_performance(self, out_path: str):
        pass

    @abstractmethod
    def calculate(self):
        pass
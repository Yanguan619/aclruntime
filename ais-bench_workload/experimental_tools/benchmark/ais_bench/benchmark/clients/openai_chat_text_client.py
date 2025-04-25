from abc import ABC

from ais_bench.benchmark.clients.base_client import BaseClient
from ais_bench.benchmark.utils import MiddleData


class OpenAIChatTextClient(BaseClient, ABC):
    def construct_request_body(
        self,
        inputs: dict,
        parameters: dict = None,
    ) -> dict:
        return inputs

    def update_middle_data(self, res: dict, inputs: MiddleData):
        try:
            generated_text = res['choices'][0]['message']['content']
        except Exception:
            generated_text = ""
        if generated_text:
            inputs.output == generated_text
            inputs.num_generated_chars = len(generated_text)
        return generated_text

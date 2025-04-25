from abc import ABC

import uuid
from ais_bench.benchmark.clients.base_client import BaseClient
from ais_bench.benchmark.utils import MiddleData


class OpenAITextClient(BaseClient, ABC):
    def construct_request_body(
        self,
        inputs: MiddleData,
        parameters: dict = None,
    ) -> dict:
        return inputs

    def update_middle_data(self, res: dict, inputs: MiddleData):
        try:
            generated_text = res['choices'][0]['text']
        except Exception as e:
            raise RuntimeError(f"Process response failed and the reason is {e}")
        if generated_text:
            inputs.output = generated_text
            inputs.num_generated_chars = len(generated_text)
        return generated_text

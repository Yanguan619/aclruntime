from abc import ABC

import uuid
from ais_bench.benchmark.clients.base_client import BaseClient
from ais_bench.benchmark.utils import MiddleData


class TritonTextClient(BaseClient, ABC):
    def construct_request_body(
        self,
        inputs: MiddleData,
        parameters: dict = None,
    ) -> dict:
        return dict(id=str(uuid.uuid4()), text_input=inputs, parameters=parameters)

    def update_middle_data(self, res: dict, inputs: MiddleData):
        generated_text = res.get("text_output", "")
        if generated_text:
            inputs.output = generated_text
            inputs.num_generated_chars = len(generated_text)
        return generated_text
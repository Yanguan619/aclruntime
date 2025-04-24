from abc import ABC
import json

from ais_bench.benchmark.clients.base_client import BaseClient
from ais_bench.benchmark.utils import MiddleData


class TGITextClient(BaseClient, ABC):
    def construct_request_body(
        self,
        inputs: MiddleData,
        parameters: dict = None,
    ) -> dict:
        return dict(inputs=inputs, parameters=parameters)

    def process_response(self, response, last_time_point):
        return json.loads(response.data.decode())

    def update_middle_data(self, res: dict, inputs: MiddleData):
        generated_text = res.get("generated_text", "")
        if generated_text:
            inputs.output = generated_text
            inputs.num_generated_chars = len(generated_text)
        return generated_text

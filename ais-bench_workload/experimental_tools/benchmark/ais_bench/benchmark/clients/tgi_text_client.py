from abc import ABC

from ais_bench.benchmark.clients.base_client import BaseClient
from ais_bench.benchmark.utils import MiddleData


class TGITextClient(BaseClient, ABC):
    def construct_request_body(
        self,
        inputs: MiddleData,
        parameters: dict = None,
    ) -> dict:
        return dict(inputs=inputs, parameters=parameters)

    def process_response()

    def update_middle_data(self, res: dict, inputs: MiddleData):
        generated_text = res.get("generated_text", "")
        if generated_text:
            inputs.output += generated_text
            inputs.num_generated_chars = len(generated_text)
        prefill_time = res.get("prefill_time")
        if prefill_time:
            inputs.prefill_latency = prefill_time
        decode_time = res.get("decode_time")
        if decode_time:
            inputs.decode_cost.append(decode_time)
        inputs.num_generated_tokens += 1
        return generated_text

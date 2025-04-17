from abc import ABC

from ais_bench.benchmark.clients.base_client import BaseStreamClient
from ais_bench.benchmark.utils import MiddleData


class OpenAIChatStreamClient(BaseStreamClient, ABC):
    def construct_request_body(
        self,
        inputs: dict,
        parameters: dict = None,
    ) -> dict:
        return inputs

    def process_stream_line(self, json_content: dict) -> dict:
        response = {}
        try:
            generated_text = json_content["choices"]["delta"]["content"]
        except Exception:
            generated_text = ""
        if generated_text:
            response.update({"generated_text": generated_text})
        return response

    def update_middle_data(self, res: dict, inputs: MiddleData):
        generated_text = res.get("generated_text", "")
        if generated_text:
            inputs.output = generated_text
            inputs.num_generated_chars = len(generated_text)
        prefill_time = res.get("prefill_time")
        if prefill_time:
            inputs.prefill_latency = prefill_time
        decode_time = res.get("decode_time")
        if decode_time:
            inputs.decode_cost.append(decode_time)
        inputs.num_generated_tokens += 1
        return generated_text

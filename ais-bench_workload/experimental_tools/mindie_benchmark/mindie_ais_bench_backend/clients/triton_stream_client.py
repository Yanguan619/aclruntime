from abc import ABC

import uuid
from ais_bench.benchmark.clients.base_client import BaseStreamClient
from ais_bench.benchmark.utils import MiddleData


class TritonStreamClient(BaseStreamClient, ABC):
    def construct_request_body(
        self,
        inputs: str,
        parameters: dict = None,
    ) -> dict:
        parameters.update({"details": True})
        return dict(id=str(uuid.uuid4()), text_input=inputs, parameters=parameters)

    def process_stream_line(self, json_content: dict) -> dict:
        response = {}
        generated_text = json_content.get("text_output", None)
        if generated_text:
            response.update({"generated_text": generated_text})
            response.update({"generated_tokens": json_content["details"].get("generated_tokens", 1)})
            response.update({"batch_size": json_content["details"].get("batch_size", 1)})
            response.update({"queue_wait_time": json_content["details"].get("queue_wait_time", 1)})
        if self.do_performance:
            response.update({"token_str": generated_text})
        return response

    def update_middle_data(self, res: dict, inputs: MiddleData):
        print(f"{res=}")
        generated_text = res.get("generated_text", "")
        if generated_text:
            inputs.output += generated_text
            inputs.num_generated_chars = len(inputs.output)
        prefill_time = res.get("prefill_time")
        if prefill_time:
            inputs.prefill_latency = prefill_time
            inputs.prefill_batch_size = res.get("batch_size", 0)
        decode_time = res.get("decode_time")
        if decode_time:
            inputs.decode_cost.append(decode_time)
            inputs.decode_batch_size.extend([res.get("batch_size", 0)] * (res.get("generated_tokens", 1) - self.last_generated_tokens))
        chunk_time_point = res.get("chunk_time_point")
        if chunk_time_point:
            inputs.chunk_time_point_list.append(chunk_time_point)
        inputs.num_generated_tokens += 1
        inputs.queue_wait_time.append(res["queue_wait_time"])
        self.last_generated_tokens = res.get("generated_tokens", 1)
        return generated_text

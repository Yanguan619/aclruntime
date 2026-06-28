# Changelog

## Unreleased

### P0 — Device-side KV Cache（零拷贝推理通道）

新增 `InferSession.run_from_tensors(feeds, out_array)`，允许以 device-resident `aclruntime.Tensor` 直接推理，跳过 H2D 传输。

**C++:**
- `TensorBase` 绑定 `.data_ptr`（返回 `GetBuffer()` 的 int64 地址）和 `.nbytes`（返回 `GetByteSize()`）
- `PyInferenceSession::RunFromTensors()` — 从 `vector<TensorBase>` 提取 `GetBuffer()/GetByteSize()` 构造 `BaseTensor`，零拷贝传给 `modelInfer_.Inference()`，返回 device 侧的 `TensorBase` 输出
- 绑定为 `session.run_from_tensors()`

**Python (ais_bench):**
- `InferSession.run_from_tensors(feeds, out_array=True)` — device tensor 输入 → 零拷贝推理 → `convert_tensors_to_host/arrays` 或原样返回

**应用 (val_qwen3p5_om.py):**
- Prefill: `infer(out_array=False)` 拿到 device TensorBase，仅 logits 下载到 host
- Decode: `run_from_tensors(out_array=False)`，所有 KV cache 全程留 device
- KV trim 回退路径：`to_host()` → numpy 裁剪 → `create_tensor_from_arrays_to_device` 回传

### P0.2 — 输入 H2D Buffer Pool

消除 `InferBaseTensorVector` 和 `FirstInnerInfer` 中每步 `aclrtMalloc` / `aclrtFree`。

- `PyInferenceSession::inputMemPool_` — 按 feed index 缓存的 `vector<MemoryData>` device 缓冲区
- `EnsureInputPool()` — 懒分配 / 按需扩容，超过现有容量才 `MxbsMalloc`
- `InferBaseTensorVector` 和 `FirstInnerInfer` 改为 `MxbsMemcpy` 到池中缓冲区后直接推理
- `Destroy()` 中遍历池释放

### P0.1 — 回退输出 Buffer Pool

`ModelInferenceProcessor` 中的 `outputMemPool_` 相关代码全部回退，恢复原有 `DestroyOutMemoryData` / `CreateOutMemoryData` / `DeInit` 行为。输出侧 TensorBase 的 `ptrData = nullptr` 机制已使 `DestroyOutMemoryData` 跳过释放。

### P2 — Python 分配振荡

- `extract_states` 返回 `List[List[type, a, b]]`（mutable list）替代 `Dict[int, Tuple]`
- `decode_inputs` step 0 预分配一次，后续 step 按 index 原地覆写

### P3 — Embedding 缓存

`Qwen35OM._embed_cache` dict，单 token decode 路径按 `tok_id` 缓存/返回 `np.ascontiguousarray` 结果。

### 杂项

- `model_process.cpp` — 外部权重加载路径提取 `cfgFail` lambda 消除重复清理；`aclmdlLoadFromFile` 路径包裹 `else` 分支；大写错误日志首字母

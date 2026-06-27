# ais-bench

A benchmarking tool for Ascend NPU inference.

## Installation

```bash
pip install .
```

## Usage

```bash
ais-bench --model /path/to/model.om --input /path/to/input
```

Or via Python module:

```bash
python -m ais_bench --model /path/to/model.om --input /path/to/input
```

## Development

```bash
pip install -e ".[dev]"
pytest
```

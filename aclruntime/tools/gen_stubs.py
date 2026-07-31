"""Regenerate aclruntime.pyi stubs from the compiled pybind11 module.

Usage (on a machine with CANN + Ascend NPU):

    # After installing the built wheel
    pip install dist/aclruntime-*.whl
    python tools/gen_stubs.py

Or directly from the build tree:

    python tools/gen_stubs.py --build-dir build
"""

import argparse
import subprocess
import sys
from pathlib import Path


def main() -> None:
    parser = argparse.ArgumentParser(description="Regenerate aclruntime pyi stubs")
    parser.add_argument(
        "--build-dir", help="CMake build directory (adds to PYTHONPATH)"
    )
    parser.add_argument(
        "--output-dir",
        help="Directory for the generated stub file (default: project root)",
    )
    args = parser.parse_args()

    project_root = Path(__file__).resolve().parent.parent
    output_dir = Path(args.output_dir).resolve() if args.output_dir else project_root

    env = dict(sys._environ) if hasattr(sys, "_environ") else {}
    if args.build_dir:
        build_dir = Path(args.build_dir).resolve()
        env.setdefault("PYTHONPATH", "")
        paths = [build_dir] + [
            p
            for p in env["PYTHONPATH"].split(":") + env.get("PYTHONPATH", "").split(";")
            if p
        ]
        sep = ";" if sys.platform == "win32" else ":"
        env["PYTHONPATH"] = sep.join(str(p) for p in set(paths))

    subprocess.check_call(
        [
            sys.executable,
            "-m",
            "pybind11_stubgen",
            "aclruntime",
            "--output-dir",
            str(output_dir),
        ],
        env=env or None,
    )

    stub_path = output_dir / "aclruntime" / "__init__.pyi"
    if stub_path.exists():
        target = output_dir / "aclruntime.pyi"
        stub_path.replace(target)
        print(f"Stubs generated: {target}")
    else:
        print("Stubs generated, but unexpected output structure.")
        print(f"Look for generated files under {output_dir / 'aclruntime'}")


if __name__ == "__main__":
    main()

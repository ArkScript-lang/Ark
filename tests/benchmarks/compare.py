import csv
import sys
from tabulate import tabulate
from dataclasses import dataclass
from pathlib import Path
from typing import List, Optional, Dict


def maybe_float(s: str) -> Optional[float]:
    if s:
        return float(s)
    return None


def maybe_str(s: str) -> Optional[str]:
    if s:
        return s
    return None


@dataclass
class Diff:
    dt_real_time: float
    dt_rt_percent: float
    dt_cpu_time: float
    dt_ct_percent: float
    time_unit: str


@dataclass
class Run:
    name: str
    iterations: int
    real_time: float
    cpu_time: float
    time_unit: str
    bytes_per_second: Optional[float] = None
    items_per_second: Optional[float] = None
    label: Optional[str] = None
    error_occurred: Optional[bool] = None
    error_message: Optional[str] = None

    def time_unit_relative(self) -> float:
        if self.time_unit == "s":
            return 1.0
        elif self.time_unit == "ms":
            return 1_000.0
        elif self.time_unit == "us":
            return 1_000_000.0
        return 1.0

    @staticmethod
    def from_dict(line: Dict):
        return Run(
            name=line["name"],
            iterations=int(line["iterations"]),
            real_time=float(line["real_time"]),
            cpu_time=float(line["cpu_time"]),
            time_unit=line["time_unit"],
            bytes_per_second=maybe_float(line["bytes_per_second"]),
            items_per_second=maybe_float(line["items_per_second"]),
            label=maybe_str(line["label"]),
            error_occurred=maybe_str(line["error_occurred"]),
            error_message=maybe_str(line["error_message"])
        )

    def diff_from_baseline(self, baseline):
        if self.time_unit != baseline.time_unit:
            raise Exception(
                f"Time units are different, can not compute a diff (baseline has {baseline.time_unit} vs {self.time_unit})")

        return Diff(
            dt_real_time=self.real_time - baseline.real_time,
            dt_rt_percent=(self.real_time - baseline.real_time) / baseline.real_time * 100.0,
            dt_cpu_time=self.cpu_time - baseline.cpu_time,
            dt_ct_percent=(self.cpu_time - baseline.cpu_time) / baseline.cpu_time * 100.0,
            time_unit=baseline.time_unit
        )


@dataclass
class Benchmark:
    name: str
    runs: List[Run]


def read_csv(file: Path):
    bench = Benchmark(file.stem, [])

    with open(file) as f:
        reader = csv.DictReader(f, delimiter=',', quotechar='"')
        for row in reader:
            bench.runs.append(Run.from_dict(row))
    return bench


def main(files: List[str]):
    paths: List[Path] = [Path(f) for f in files]
    benchmarks: List[Benchmark] = [read_csv(path) for path in paths]

    headers = ["", ""] + [b.name for b in benchmarks]
    runs_by_name = {}
    for b in benchmarks:
        for run in b.runs:
            if run.name not in runs_by_name:
                runs_by_name[run.name] = []
            runs_by_name[run.name].append(run)

    data = []
    for (name, runs) in runs_by_name.items():
        baseline = runs[0]
        diffs = [r.diff_from_baseline(baseline) for r in runs[1:]]
        data.append(
            [
                name,
                "real_time\ncpu_time",
                f"{baseline.real_time}{baseline.time_unit}\n{baseline.cpu_time}{baseline.time_unit}"
            ] + [
                f"{diff.dt_real_time:.3f} ({diff.dt_rt_percent:.4f}%)\n{diff.dt_cpu_time:.3f} ({diff.dt_ct_percent:.4f}%)"
                for
                diff in diffs
            ])
    print(tabulate(data, headers, tablefmt="presto"))


if __name__ == "__main__":
    if len(sys.argv) > 2:
        main(sys.argv[1:])
    else:
        print("compare.py needs at least two csv files: a baseline and one (or more) to compare against")

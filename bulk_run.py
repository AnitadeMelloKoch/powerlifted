import subprocess
import os
import random
from pathlib import Path
import argparse

RUN_COMMANDS = [
    "--fd-search", "lazy_greedy([ff()], preferred=[ff()])",
    "--process-mem-limit", "4G",
    "--process-timeout", "600",
    "--time-limit", "1800",
]

def find_domain_and_problems(folder_dir):
    """Flat layout: one domain.pddl + problems in the same folder (IPC style)."""
    domain = None
    problems = []
    for file in Path(folder_dir).glob("*.pddl"):
        if "domain.pddl" in file.name:
            domain = file
        else:
            problems.append(file)
    return domain, problems


def find_domain_problem_pairs(folder_dir):
    """Nested layout: each problem lives alongside its own domain file (minecraft style)."""
    pairs = []
    for file in Path(folder_dir).glob("**/*.pddl"):
        if "domain.pddl" not in file.name:
            siblings = list(file.parent.glob("*domain.pddl"))
            if siblings:
                pairs.append((siblings[0], file))
    return pairs


def is_flat_layout(folder):
    """Detect IPC-style layout by checking if any immediate subfolder has a domain.pddl directly."""
    return any(
        "domain.pddl" in p.name
        for f in Path(folder).glob("*/")
        for p in f.glob("*.pddl")
    )

def run_planner(domain, problem, outdir, fd, power, cost, use_random, processes, disable_links, lifted_name, seed):
    assert not (fd and power)
    assert not (cost and use_random)
    
    os.makedirs(outdir, exist_ok=True)
    
    cmd = [
        "python", "powerlifted.py", "-d", str(domain),
        "-i", str(problem), "--build", "--speculative", 
        "--cxx-compiler", "/usr/local/bin/mpic++",
        "--save-folder", outdir,
        "--translator-output-file", lifted_name,
        "--seed", str(seed)
    ] + RUN_COMMANDS
    
    if fd:
        cmd += ["--fd"]
    
    if cost:
        cmd += ["--scope", "cost"]
    
    if use_random:
        cmd += ["--scope", "random"]
    if disable_links:
        cmd += ["--turn-link-off"]
    
    cmd += ["--processes", str(processes)]
    
    print(cmd)
    
    result = subprocess.run(cmd)
    
    return result.returncode

def validate_plan(domain, problem, out_dir):
    plans = []
    for file in Path(out_dir).glob("*plan*"):
        plans.append(str(file))
    
    cmd = ["Validate", str(domain), str(problem)] + plans
    result = subprocess.run(cmd, capture_output=True, text=True)
    with open(os.path.join(out_dir, "validate.log"), "w") as f:
        f.write(result.stdout)
        f.write("\n--- STDERR ---\n")
        f.write(result.stderr)
    
    return result.returncode == 0

def main(folder, fd, power, cost, use_random, processes, disable_links, save_folder, lifted_name, seed):
    if is_flat_layout(folder):
        pairs = []
        for f in Path(folder).glob("*"):
            domain, problems = find_domain_and_problems(f)
            if domain is None:
                print(f"Skipping {f} — no domain.pddl found")
                continue
            for problem in problems:
                pairs.append((domain, problem))
    else:
        pairs = find_domain_problem_pairs(folder)

    random.shuffle(pairs)

    for domain, problem in pairs:
        problem_name = problem.stem
        relative_subdir = problem.parent.relative_to(folder)
        subdir_str = "_".join(relative_subdir.parts)
        out_dir = os.path.join(save_folder, subdir_str, problem_name)

        summary_file = Path(out_dir) / "summary.out"
        if summary_file.is_file():
            continue

        return_code = run_planner(domain, problem, out_dir, fd, power, cost,
                                  use_random, processes, disable_links, lifted_name, seed)

        if return_code == 0:
            valid = validate_plan(domain, problem, out_dir)
            print(valid)
        else:
            print("Planner failed")



if __name__ == "__main__":    
    parser = argparse.ArgumentParser()
    
    parser.add_argument('-f', '--folder', dest='folder')
    parser.add_argument('--fd', action='store_true')
    parser.add_argument('--power', action='store_true')
    parser.add_argument('--random', action='store_true')
    parser.add_argument('--cost', action='store_true')
    parser.add_argument('-s', '--seed', action="store", type=int, default=0)
    parser.add_argument('-p', '--processes', dest='processes')
    parser.add_argument('--disable-links', action='store_true', dest="disable_links")
    parser.add_argument('--save_dir', action="store", default="experiment", type=str)
    parser.add_argument('--lifted', action="store", default="exp.lifted")
    
    
    args = parser.parse_args()

    main(args.folder,
         args.fd,
         args.power,
         args.cost,
         args.random,
         args.processes,
         args.disable_links,
         args.save_dir,
         args.lifted,
         args.seed)
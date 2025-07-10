import subprocess 
import os 
from pathlib import Path 
import argparse

RUN_COMMANDS = [
    "--fd-search", "astar (lmcut())"
]

def find_domain_and_problems(folder_dir):
    domain = None 
    problems = []
    for file in Path(folder_dir).glob("*.pddl"):
        if file.name == "domain.pddl":
            domain = file
        else:
            problems.append(file)
    
    return domain, problems

def run_planner(domain, problem, outdir, fd, power, cost, random, processes):
    assert not (fd and power)
    assert not (cost and random)
    
    os.makedirs(outdir, exist_ok=True)
    
    cmd = [
        "python", "powerlifted.py", "-d", str(domain),
        "-i", str(problem), "--build", "--speculative", 
        "--cxx-compiler", "/usr/local/bin/mpic++",
        "--save-folder", outdir
    ] + RUN_COMMANDS
    
    if fd:
        cmd += ["--fd"]
    
    if cost:
        cmd += ["--scope", "cost"]
    
    if random:
        cmd += ["--scope", "random"]
    
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

def main(folder, fd, power, cost, random, processes):
    domain, problems = find_domain_and_problems(folder)
    assert domain is not None, "No domain.pddl found. Script does not support problems with domain file not matching domain.pddl"
    assert len(problems) > 0, "No problems found."
    
    for problem in problems:
        problem_name = problem.stem
        out_dir = os.path.join("experiments", Path(folder).name, problem_name)
        
        return_code = run_planner(domain, 
                                  problem, 
                                  out_dir,
                                  fd,
                                  power,
                                  cost,
                                  random,
                                  processes)
        
        if return_code == 0:
            valid = validate_plan(domain, problem, out_dir)
            print(valid)
        else:
            print("Planner failed")

if __name__ == "__main__":
    import sys 
    
    parser = argparse.ArgumentParser()
    
    parser.add_argument('-f', '--folder', dest='folder')
    parser.add_argument('--fd', action='store_true')
    parser.add_argument('--power', action='store_true')
    parser.add_argument('--random', action='store_true')
    parser.add_argument('--cost', action='store_true')
    parser.add_argument('-p', '--processes', dest='processes')
    
    args = parser.parse_args()
    
    main(args.folder,
         args.fd,
         args.power,
         args.cost,
         args.random,
         args.processes)
import subprocess 
import os 
from pathlib import Path 
import argparse

RUN_COMMANDS = [
    "--fd-search", "lazy_greedy([ff()], preferred=[ff()])",
    "--process-mem-limit", "4G",
    "--process-timeout", "600",
    "--time-limit", "1800",
]

def find_domain_and_problems(folder_dir):
    domain = None 
    problems = []
    for file in Path(folder_dir).glob("**/*.pddl"):
        if "domain.pddl" in file.name:
            # if domain is not None:
            #     return None, problems
            domain = file
        else:
            problems.append(file)
    
    return domain, problems

# def find_domain_and_problems(folder_dir):
#     domain = [] 
#     problems = []
#     for file in Path(folder_dir).glob("**/*.pddl"):
#         if "domain.pddl" in file.name:
#             domain.append(file)
#         else:
#             problems.append(file)
    
#     return domain, problems

def run_planner(domain, problem, outdir, fd, power, cost, random, processes, disable_links, lifted_name, seed):
    assert not (fd and power)
    assert not (cost and random)
    
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
    
    if random:
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

def main(folder, fd, power, cost, random, processes, disable_links, save_folder, lifted_name, seed):
    for f in Path(folder).glob("*"):
        domain, problems = find_domain_and_problems(f)
        # domains, problems = find_domain_and_problems(f)
        # assert domain is not None, "No domain.pddl found. Script does not support problems with domain file not matching domain.pddl"
        
        for problem in problems:
        # for domain, problem in zip(domains,problems):
            problem_name = problem.stem
            
            relative_subdir = problem.parent.relative_to(folder)
            subdir_str = "_".join(relative_subdir.parts)
            
            out_dir = os.path.join(save_folder, subdir_str, problem_name)
            
            summary_file = Path(out_dir) / "summary.out"
            if summary_file.is_file():
                continue 
            
            return_code = run_planner(domain, 
                                    problem, 
                                    out_dir,
                                    fd,
                                    power,
                                    cost,
                                    random,
                                    processes,
                                    disable_links,
                                    lifted_name,
                                    seed)
            
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
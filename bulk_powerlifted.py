import subprocess 
import os 
from pathlib import Path 
import argparse
from concurrent.futures import ThreadPoolExecutor


RUN_COMMANDS = [
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

def run_planner(domain, problem, outdir):
    
    os.makedirs(outdir, exist_ok=True)
    
    out_file = outdir + "/" + "output.txt"
    
    cmd = [
        "python", "powerlifted.py", "-d", str(domain),
        "-i", str(problem), 
        "--translator-output-file", f"{problem.stem}.lifted",
    ] + RUN_COMMANDS
    
    print(cmd)
    
    with open(out_file, 'w') as f:
        result = subprocess.run(cmd, stdout=f, stderr=subprocess.STDOUT, text=True)
    
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

def main(folder):
    all_problems = []
    for f in Path(folder).glob("*"):
        domain, problems = find_domain_and_problems(f)
        if domain is None:
            print(f"Skipping {f} — missing or multiple domain files.")
            continue
        
        for problem in problems:
            problem_name = problem.stem
            
            relative_subdir = problem.parent.relative_to(folder)
            subdir_str = "_".join(relative_subdir.parts)
            
            outdir = os.path.join("experiments", "powerlifted_dup", subdir_str, problem_name)
            
            all_problems.append((domain, problem, outdir))

    with ThreadPoolExecutor(max_workers=32) as executor:
        futures = [executor.submit(run_planner, dom, prob, outdir) for dom, prob, outdir in all_problems]



if __name__ == "__main__":    
    parser = argparse.ArgumentParser()
    
    parser.add_argument('-f', '--folder', dest='folder')
    
    
    args = parser.parse_args()

    main(args.folder)
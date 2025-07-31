import subprocess 
import os 
from pathlib import Path 
import argparse

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

def run_planner(domain, problem, outdir, lifted_name, seed):
    
    os.makedirs(outdir, exist_ok=True)
    
    out_file = outdir + "/" + "output.txt"
    
    cmd = [
        "python", "powerlifted.py", "-d", str(domain),
        "-i", str(problem), "--build",
        "--save-folder", outdir,
        "--translator-output-file", lifted_name,
        "--seed", str(seed)
    ] + RUN_COMMANDS
    
    print(cmd)
    
    with open(out_file, 'w') as f:
        result = subprocess.run(cmd, stdout=f, text=True)
    
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

def main(folder, save_folder, lifted_name, seed):
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
    parser.add_argument('-s', '--seed', action="store", type=int, default=0)
    parser.add_argument('--save_dir', action="store", default="experiment", type=str)
    parser.add_argument('--lifted', action="store", default="exp.lifted")
    
    
    args = parser.parse_args()

    main(args.folder,
         args.save_dir,
         args.lifted,
         args.seed)
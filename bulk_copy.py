import subprocess 
from pathlib import Path
import argparse 
import os

def find_domains_and_problems(folder_dir):
    domain = None
    problems = []
    for file in Path(folder_dir).glob("*.pddl"):
        if file.name == "domain.pddl":
            domain = file
        else:
            problems.append(file)
    return domain, problems

def run_duplicator(domain, problem, outdir, prefix):
    os.makedirs(outdir, exist_ok=True)
    outfile = outdir + "/" + problem.stem + "_" + prefix + ".pddl"
    
    cmd = [
        "python", "powerlifted.py", "-d", domain,
        "-i", problem, "--build", "--cxx-compile",
        "/usr/local/bin/mpic++", "--pddl-file", outfile,
        "--duplicate-file", prefix
    ]
    
    print(cmd)
    
    try:
        result = subprocess.run(cmd, timeout=300)
    except subprocess.TimeoutExpired:
        print(f"Timeout expired for {outfile}. Skipping.")
        result = None
     
    return result, outfile
    
def main(benchmark_folder):
    for folder in Path(benchmark_folder).glob("*"):
        domain, problems = find_domains_and_problems(folder)
        if domain is None:
            print("No single domain found in", folder.name, "skipping duplication...")
            continue
        
        for problem in problems:
            outdir = os.path.join("benchmarks", "dup-domains", folder.stem)
            _, file1 = run_duplicator(domain, problem, outdir, "dup1")
            _, file1 = run_duplicator(domain, Path(file1), outdir, "dup2")
            _, file1 = run_duplicator(domain, Path(file1), outdir, "dup3")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    
    parser.add_argument('-f', '--folder', dest='folder')
    args = parser.parse_args()
    
    main(args.folder)



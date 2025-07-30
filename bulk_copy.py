import subprocess 
from pathlib import Path
import argparse 
import os
from concurrent.futures import ThreadPoolExecutor

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
        "-i", problem, "--cxx-compile",
        "/usr/local/bin/mpic++", "--pddl-file", outfile,
        "--duplicate-file", prefix
    ]
    
    print(cmd)
    
    try:
        result = subprocess.run(cmd, timeout=120)
    except subprocess.TimeoutExpired:
        print(f"Timeout expired for {outfile}. Skipping.")
        result = None
     
    return result, outfile

def run_triple(domain, problem, outdir):
    _, file = run_duplicator(domain, problem, outdir, "dup1")
    _, file = run_duplicator(domain, Path(file), outdir, "dup2")
    _, file = run_duplicator(domain, Path(file), outdir, "dup3")
    _, file = run_duplicator(domain, Path(file), outdir, "dup4")

def main(benchmark_folder):
    all_problems = []
    for f in Path(benchmark_folder).glob("*"):
        domain, problems = find_domains_and_problems(f)
        if domain is None:
            print(f"Skipping {f} — missing or multiple domain files.")
            continue
        out_dir = os.path.join("benchmarks", "dup-domains", f.stem)
        for problem in problems:
            all_problems.append((domain, problem, out_dir))
    
    with ThreadPoolExecutor(max_workers=1) as executor:
        futures = [executor.submit(run_triple, domain, prob, out_dir) for domain, prob, out_dir in all_problems]


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    
    parser.add_argument('-f', '--folder', dest='folder')
    args = parser.parse_args()
    
    main(args.folder)



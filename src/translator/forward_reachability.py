import normalize
import pddl_parser


def main():
    domain_file = ""
    task_file = ""
    
    task = pddl_parser.open(domain_filename=domain_file,
                            task_filename=task_file)
    normalize.normalize(task)


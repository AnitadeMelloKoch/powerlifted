# Tentative Object Pruning

Code accompanying the anonymous submission *"Efficient Search by Tentatively
Pruning Objects from Planning Tasks."* Distribution, citation, or public
sharing of this repository is restricted while the submission is under
review; see the paper for details.

Tentative object pruning treats object selection as a sampling problem: it
builds a sequence of simplified planning tasks over reduced, candidate
object subsets — always preserving the original goal — and searches them in
parallel until one yields a plan valid in the original task. This repository
implements it as `--speculative` mode on top of a forked copy of the
[Powerlifted](https://github.com/abcorrea/powerlifted) lifted planner (see
[References](#references)), and can dispatch each candidate task to either
Powerlifted itself or Fast Downward.

## Requirements
- A C++17-compliant compiler
- CMake 3.9+
- Boost (`program_options`)
- An MPI implementation (e.g. OpenMPI) and its compiler wrapper (e.g. `mpic++`) — required for `--speculative`, which coordinates worker processes over MPI
- Python 3.5+
- To dispatch candidate tasks to Fast Downward (`--fd`), a Fast Downward checkout built into `fd-builds/release/bin`

## Build

```
python powerlifted.py -d DOMAIN -i INSTANCE --build --cxx-compiler mpic++ [...]
```

or build ahead of time with `build.py`:

```
python build.py --cxx-compiler mpic++
```

`--cxx-compiler` must point at your MPI compiler wrapper (or its full path)
since the search component links against MPI directly.

## Running speculative scoping

Because `--speculative` spawns MPI worker processes, invoke it through
`mpirun`:

```
mpirun -n 5 python powerlifted.py -d dev/domains/test/blocks/domain.pddl \
    -i dev/domains/test/blocks/blocks_50.pddl \
    --build --cxx-compiler mpic++ \
    --speculative --scope cost --processes 4
```

`-n 5` = 1 coordinator process + `--processes 4` workers, each independently
searching a different candidate object set until one finds a plan valid in
the original task, or every candidate has been exhausted.

### Options
- `--speculative`: scope the task speculatively before search, instead of solving it directly.
- `--scope {cost,random}` (default `cost`): candidate object set construction strategy.
  - `cost`: greedily grow a per-type object quota starting from the objects with the lowest linking cost (fewest additional objects pulled in by the initial-state relevant-link closure), evaluating the narrowest object sets first — analogous to iterative deepening over problem breadth.
  - `random`: sample a random number of objects per relevant type on each attempt.
- `--turn-link-off`: skip the relevant-link closure that pulls in objects co-occurring with selected objects in the initial state. This allows smaller, more aggressive candidate sets, but any plan found must be validated against the original task before being accepted ("unsafe" pruning) rather than being valid by construction ("safe" pruning).
- `--processes N` (default `4`): number of MPI worker processes searching candidate object sets in parallel.
- `--process-timeout SECONDS` (default `1800`): per-worker time limit for a single candidate object set.
- `--process-mem-limit LIMIT` (default `128G`, e.g. `4G`): per-worker memory limit.
- `--fd`: dispatch each candidate task to Fast Downward instead of Powerlifted; combine with `--fd-search "SEARCH_STRING"` to pass a search configuration to FD.
- `--save-folder DIR`: where the plan and a `summary.out` (number of candidate scopes tried, whether a plan was found, wall time, and total CPU time summed across all workers) are written.

Every other `powerlifted.py` flag (search algorithm, evaluator, successor
generator, etc.) still applies to the underlying search used on each
candidate task; run `python powerlifted.py --help` for the full list.

## Experiment scripts

These scripts drive the batch experiments used to evaluate object pruning
across a benchmark suite. All of them accept a benchmark folder in either a
flat IPC-style layout (`domain.pddl` + problem files in the same directory)
or a nested per-problem layout (each problem alongside its own
`*domain.pddl`).

- `bulk_copy.py`: given a benchmark folder, duplicates each problem's object
  set and replicates the corresponding initial-state atoms, producing
  problems with inflated object counts and an unchanged goal — used to build
  the expanded-IPC benchmarks.
- `bulk_powerlifted.py`: runs the unmodified Powerlifted baseline (no
  speculative scoping) over a benchmark folder.
- `bulk_run.py`: runs the full sweep (Fast Downward or Powerlifted, cost-based
  or random scoping, with or without the relevant-link closure) over a
  benchmark folder, one subprocess per problem.
- `bulk_count.py`: runs `--speculative` scoping (default options) over a
  benchmark folder and saves the resulting scoped PDDL file per problem, for
  offline inspection of the candidate object sets chosen.

## Limitations
When a candidate task is searched with Powerlifted itself (the default, i.e.
without `--fd`), the following PDDL features are not supported:
- **Axioms**
- **Conditional effects**
- **Negated preconditions**: only inequality
- **Quantifiers**

These do not apply when dispatching candidate tasks to Fast Downward
(`--fd`).

## References

1. Corrêa, A. B.; Pommerening, F.; Helmert, M.; and Francès, G. 2020. Lifted Successor Generation using Query Optimization Techniques. In Proc. ICAPS 2020, pp. 80-89. [[pdf]](https://ai.dmi.unibas.ch/papers/correa-et-al-icaps2020.pdf)
2. Corrêa, A. B.; Francès, G.; Pommerening, F.; and Helmert, M. 2021. Delete-Relaxation Heuristics for Lifted Classical Planning. In Proc. ICAPS 2021, pp. 94-102. [[pdf]](https://ai.dmi.unibas.ch/papers/correa-et-al-icaps2021.pdf)
3. Corrêa, A. B.; Pommerening, F.; Helmert, M.; and Francès, G. 2022. The FF Heuristic for Lifted Classical Planning. In Proc. AAAI 2022. [[pdf]](https://ai.dmi.unibas.ch/papers/correa-et-al-aaai2022.pdf)
4. Corrêa, A. B.; and Seipp, J. 2022. Best-First Width Search for Lifted Classical Planning. In Proc. ICAPS 2022. [[pdf]](https://ai.dmi.unibas.ch/papers/correa-seipp-icaps2022.pdf)
5. Corrêa, A. B.; Frances, G.; Hecher, M.; Longo, D. M.; and Seipp, J. 2023. The powerlifted planning system in the IPC 2023. Tenth International Planning Competition (IPC-10): Planner Abstracts.

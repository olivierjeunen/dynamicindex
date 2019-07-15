# Dynamic Index (RecSys '19)

C++ source code for the Dynamic Index algorithm proposed in "Efficient Similarity Computation for Collaborative Filtering in Dynamic Environments", as appeared in the proceedings of the 2019 ACM International Conference on Recommender Systems.


## Requirements and Installation
OpenMP is required for parallellisation.
Instructions for compilation using CMake are as follows:

```
$ cd dynamicindex/src
$ mkdir build; cd build
$ cmake ..
$ make all
```

## Data Format
All experiments expect a .csv-file containing user-item-timestamp triplets, including a header.
For the incremental experiments, we expect the file to be ascendingly sorted on the timestamp-column.

## Experiments
#### Baselines (`master`)
To compare Dynamic Index with baseline methods, run the following command:
```
./Baseline_Experiments <name> <pageviews> <batch_size> <num_threads>
```
- `<name>` is an identifier for the experiment, results will be written to an output file containing this identifier>
- `<pageviews>` is the .csv-file containing user-item-timestamp triplets
- `<batch_size>` is the amount of interactions after which similarities are recomputed
- `<num_threads>` is the number of threads used by OMP to parallellise the computation.

This corresponds to Figure 3 in the paper.

#### Parallellisation (`master`)
To run Dynamic Index on a given dataset, run the following command:
```
./Incremental_Experiments <name> <pageviews> <batch_size> <num_threads>
```
Command-line arguments are described above.

This corresponds to Figure 4 in the paper, with varying `<num_threads>`.

#### Recommendability (`recommendability`)
To run Dynamic index on a given dataset, with restrictions on the recommendability of items, run the following command:
```
./Recommendability_Experiments <name> <pageviews> <item_origins> <batch_size> <num_threads> <recommendability_seconds>
```
- `<item_origins>` is a .csv-file containing items first occurrences', which can be computed through `python3 ComputeFirstItemOccurrences.py <pageviews>`
- `<recommendability_seconds>` is the number of seconds an item remains recommendable after its first occurrence, denoted as delta in the paper
The rest of the command-line arguments are described above.

This corresponds to Figure 5 in the paper, with varying delta.

## Notes
The `master` branch does not include all the functionality needed for environments with restricted recommendability, as this imposes some unnecessary overhead when all items are recommendable.
Code and experiments for this setting can be found in the `recommendability` branch.

Abstract for the paper:
> "The problem of computing all pairwise similarities in a large collection of vectors is a well-known and common data mining task.
> As the number and dimensionality of these vectors keeps increasing, however, currently existing approaches are often unable to meet the strict efficiency requirements imposed by the environments they need to perform in.
Real-time neighbourhood-based collaborative filtering (CF) is one example of such an environment in which performance is critical.
> In this work, we present a novel algorithm for efficient and exact similarity computation between sparse, high-dimensional vectors.
> Our approach exploits the sparsity that is inherent to implicit feedback data-streams, entailing significant gains compared to other methods.
> Furthermore, as our model learns incrementally, it is naturally suited for dynamic real-time CF environments.
> We propose a MapReduce-inspired parallellisation procedure along with our method, and show how even more speed-up can be achieved.
> Additionally, in many real-world systems, many items are actually not \emph{recommendable} at any given time, due to recency, stock, seasonality, or enforced business rules.
> We exploit this fact to further improve the computational efficiency of our approach.
> Experimental evaluation on both real-world and publicly available datasets shows that our approach scales up to millions of processed user-item interactions per second, and well advances the state-of-the-art."

If you use this implementation in a research project, please cite the accompanying paper:
>Jeunen, O., Verstrepen, K. and Goethals, B. "Efficient Similarity Computation for Collaborative Filtering in Dynamic Environments." In Proceedings of the 13th ACM Conference on Recommender Systems (RecSys '19). ACM, 2019.

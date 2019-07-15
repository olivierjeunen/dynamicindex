# Dynamic Index (RecSys '19)

C++ source code for the Dynamic Index algorithm proposed in "Efficient Similarity Computation for Collaborative Filtering in Dynamic Environments", as appeared in the proceedings of the 2019 ACM International Conference on Recommender Systems.

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

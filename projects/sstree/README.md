# **Similarity Search Tree**

R-Trees
- splitting heuristic
- criteria used to choose the sub-tree to add new points
- distance metric

SSTree
- inserting criteria: reduce the variance of each of the newly created nodes
    - choose the dim with the highest variance
    - splits the sorted list of children to reduce variance along that dimension

- centroid is defined as the center of mass of a set of points, whose coordinates are the weighted sum of the point's coordinates.
    - is leaf: centroid of the points belonging to it.
    - is internal node: center of mass of the centroids of its children nodes

Depth-First KNN

better performance could be obtained by not visiting every element and its objects when it can be determined that it is impossible for an element to contain any of the K-nearest neighbors of q

Skip if `d(q, e) <= d(q, e_p)` for every `e_p` in `e` and `d(q, e) > D_k` is satisfied, where `e` is a nonobject element.

Define `M_p` the mean and the distance `r_p,max` from `M_p` to the farthest object within `e_p`
`M_p` is also known as pivot or routing object
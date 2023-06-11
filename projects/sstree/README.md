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
import math
import random
import argparse
import traceback
from Mtree import MTree
from tqdm import tqdm

def euclidean_distance(point1, point2):
    return math.sqrt(sum([(a - b) ** 2 for a, b in zip(point1, point2)]))

def main(n_points, max_size):
    # Genera n_points puntos aleatorios
    points = [(random.uniform(0, 100), random.uniform(0, 100)) for _ in range(n_points)]
    
    # Crea un M-tree
    tree = MTree(euclidean_distance, max_size)

    # Inserta los puntos en el M-tree
    print("Inserting points...")
    i = 0
    for point in tqdm(points):
        try:
            # print(f"\nInserting point {i}: {point}")
            tree.insert(point)
        except Exception as e:
            tree.print()
            print(f"Error inserting {i}-th point")
            traceback.print_exc()
            break
        i+=1

    # print('\n\n\n')
    print()
    tree.print()

    # # Realizar una consulta de rango
    # query_point = (50, 50)
    # radius = 10
    # results = tree.range_query(query_point, radius)
    # print(f"Points within {radius} of {query_point}: {results}")

    # # Realizar una consulta de los K vecinos más cercanos
    # k = 5
    # results = tree.kNN(query_point, k)
    # print(f"The {k} nearest neighbors to {query_point}: {results}")
    
    # Verifica el árbol
    print(f"Tree check result: {tree.checkTree()}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Generate M-tree with n random points.')
    parser.add_argument('--n_points', type=int, default=500, help='Number of points to generate')
    parser.add_argument('--max_size', type=int, default=  20, help='Maximum size of a node')

    args = parser.parse_args()
    main(args.n_points, args.max_size)
import inspect
import numpy as np
from scipy.spatial import distance
import pickle
import os
from typing import List
from queue import PriorityQueue
from joblib import dump, load

mm = None
MM = None

class SSnode:
    # Inicializa un nodo de SS-Tree
    def __init__(self, leaf=False, points=None, children=None, data=None, parent=None):
        self.leaf : bool            = leaf
        self.parent : SSnode        = parent

        self.points : List[np.array]   = points if points is not None else []
        self.children : List[SSnode]= children if children is not None else []
        self.data : List[str]       = data if data is not None else []

        self.centroid : np.array    = np.mean([p for p in self.points], axis=0) if self.points else None
        self.radius : np.float64    = self.compute_radius()

    # Calcula el radio del nodo como la máxima distancia entre el centroide y los puntos contenidos en el nodo
    def compute_radius(self) -> np.float64:
        """recompute radius"""
        print(inspect.currentframe().f_code.co_name)
        if not self.points:
            return 0.0

        distances = []
        if self.leaf:
            distances = list(map(lambda p: distance.euclidean(self.centroid, p), self.points))
        else:
            distances = list(map(lambda child: distance.euclidean(self.centroid, child.centroid) + child.radius, self.children))
        return max(distances)

    # Verifica si un punto dado está dentro del radio del nodo
    def intersects_point(self, point) -> bool:
        """check if point is inside the node's radius"""
        print(inspect.currentframe().f_code.co_name)
        return distance.euclidean(self.centroid, point) <= self.radius

    # Actualiza el envolvente delimitador del nodo recalculando el centroide y el radio
    def update_bounding_envelope(self) -> None:
        """update the bounding envelope of the node (update the k-dimensional sphere)"""
        print(inspect.currentframe().f_code.co_name)
        self.centroid = np.mean(self.get_entries_centroids(), axis=0)
        self.radius = self.compute_radius()

    # Encuentra y devuelve el hijo más cercano al punto objetivo
    # Se usa para entrar el nodo correto para insertar un nuevo punto
    def find_closest_child(self, target):
        """find the closest child to the target point"""
        print(inspect.currentframe().f_code.co_name)
        if self.leaf:
            raise Exception("No se puede encontrar el hijo más cercano de un nodo hoja.")
        
        min_dist = np.inf
        closest_child = None
        for child_node in self.children:
            # print(f'child_node.centroid: {child_node.centroid}')
            curr_dis = distance.euclidean(child_node.centroid, target)
            if curr_dis < min_dist:
                min_dist = curr_dis
                closest_child = child_node
        
        return closest_child

    # Divide el nodo en dos a lo largo del eje de máxima varianza
    def split(self):
        """split the node by the axis of maximum variance, minimizing the variance of the new nodes"""
        print(inspect.currentframe().f_code.co_name)
        split_index = self.find_split_index()
        new_child_1 = None
        new_child_2 = None

        if self.leaf:
            new_child_1 = SSnode(leaf=True, points=self.points[0:split_index], data=self.data, parent=self)
            new_child_2 = SSnode(leaf=True, points=self.points[split_index:], data=self.data, parent=self)
        else:
            new_child_1 = SSnode(leaf=False, children=self.children[0:split_index], parent=self)
            new_child_2 = SSnode(leaf=False, children=self.children[split_index:], parent=self)
        
        return new_child_1, new_child_2

    # Encuentra el índice en el que dividir el nodo para minimizar la varianza total
    def find_split_index(self):
        """find the index to split the node minimizing the total variance using the axis of the max variance"""
        print(inspect.currentframe().f_code.co_name)
        coordinate_index = self.direction_of_max_variance()
        sorted_points = sorted(self.get_entries_centroids(), key=lambda p: p[coordinate_index])
        return self.min_variance_split([p[coordinate_index] for p in sorted_points])

    # Encuentra la división que minimiza la varianza total
    def min_variance_split(self, values):
        """obtain the index that minimizes the total variance using brownie split"""
        print(inspect.currentframe().f_code.co_name)
        split_index = []
        for _ in range(mm, len(values) - mm + 1):     # [>=, <]
            sum_variances = np.var(values[:_]) + np.var(values[_:])
            split_index.append((_, sum_variances))
        return min(split_index, key=lambda x: x[1])[0]


    # Encuentra el eje a lo largo del cual los puntos tienen la máxima varianza
    def direction_of_max_variance(self) -> int:
        """find the axis in the k-dimensional space of the maximum variance"""
        print(inspect.currentframe().f_code.co_name)
        centroids = self.get_entries_centroids()
        variances = np.var(centroids, axis=0)
        idx_max = list(variances).index(max(variances))
        print(f'variances: {variances} {idx_max}')
        return idx_max


    # Obtiene los centroides de las entradas del nodo
    def get_entries_centroids(self) -> List[np.array]:
        """utility function to get the centroids of the entries of the node"""
        print(inspect.currentframe().f_code.co_name)
        if self.leaf:
            return self.points
        else:
            return [child.centroid for child in self.children]
    
    # Inserta un punto en el árbol
    def insert(self, point, data=None):
        """inserts a point(with the data) in the tree"""
        print(inspect.currentframe().f_code.co_name)
        if self.leaf:
            if point in self.points:
                return (None, None)
            
            self.points.append(point)
            self.data.append(data)
            
            self.update_bounding_envelope()

            if len(self.points) <= MM:
                return (None, None)
        else:
            closest_child = self.find_closest_child(point)
            (new_child_1, new_child_2) = closest_child.insert(point, data)
            if new_child_1 is not None and new_child_2 is not None:
                idx_to_remove = self.children.index(closest_child)
                # remove by index
                self.children.pop(idx_to_remove)
                self.data.pop(idx_to_remove)

                self.children.append(new_child_1)
                self.children.append(new_child_2)
                # new_child_1.parent = self
                # new_child_2.parent = self
                self.update_bounding_envelope()

                if len(self.children) <= MM:
                    return (None, None)
            else:
                self.update_bounding_envelope()
                return (None, None)
        
        return self.split()
                
    def printNode(self, indent=0):
        print('\t' * indent, f'Centroid: {self.centroid}, Radius: {round(self.radius, 5)}, Size: {len(self.points)}')
        for child in self.children:
            child.printNode(indent + 2)


class SSTree:
    # Inicializa un SS-Tree
    def __init__(self, M=None, m=None, filename=None):
        if filename is None:
            self.M = M
            self.m = m

            global mm, MM
            mm = m
            MM = M

            if M is not None and m is not None:
                self.root = SSnode(leaf=True)
            else:
                self.root = None
        else:
            if os.path.exists(filename):
                loaded_tree = self.load(filename)
                self.M = loaded_tree.M
                self.m = loaded_tree.m
                self.root = loaded_tree.root
            else:
                print(f"'{filename}' no existe.")
                self.M = None
                self.m = None
                self.root = None


    # Inserta un punto en el árbol
    def insert(self, point, data=None):
        (new_child_1, new_child_2) = self.root.insert(point, data)
        if new_child_1 is not None and new_child_2 is not None:
            self.root = SSnode(leaf=False, children=[new_child_1, new_child_2])

    
    # Busca un punto en el árbol y devuelve el nodo que lo contiene si existe
    def search(self, target):
        return self._search(self.root, target)


    # Función recursiva de ayuda para buscar un punto en el árbol
    def _search(self, node, target):
        """searches exactly point in the tree"""
        if node.leaf:
            return node if target in node.points else None
        else:
            for child in node.children:
                if child.intersects_point(target):
                    result = self._search(child, target)
                    if result is not None:
                        return result
        return None
    

    # Depth-First K-Nearest Neighbor Algorithm
    def knn(self, q, k=3):
        # Completar aqui!
        pass

    
    # Guarda el árbol en un archivo
    def save(self, filename):
        with open(filename, 'wb') as f:
            dump(self, f)


    # Carga un árbol desde un archivo
    @staticmethod
    def load(filename):
        with open(filename, 'rb') as f:
            return load(f)


    def print(self):
        if self.root is not None:
            self.root.printNode()
        else:
            print("El árbol está vacío.")


if __name__ == "__main__":
    ss = SSTree(M=6, m=2)    # , filename='tree.ss'
    print(mm, MM)
    data = [
        [0.00438936,-0.00058534,0.00174215,0.00744667],
        [0.000172,0.00704932,0.00117934,0.03026242],
        [1.2345414e-04,-7.0645498e-08,3.5576265e-02,4.4623464e-03],
        [0.0009176,0.01680653,0.059632,0.00065029],
        [0.0008271,0.01525626,0.04283572,0.00923643],
        [0.00082534,-0.00039568,0.03403273,0.05716665],
        [-4.7112128e-04,-1.0372750e-03,-6.1829771e-05,4.8588943e-03],
        [1.0265931e-02,1.4486914e-02,-8.5602907e-05,6.0707338e-02],
        [-0.00012903,-0.0006901,0.06522365,0.01314898],
        [-1.2266746e-07,1.8020669e-02,3.7103205e-04,1.3928285e-02],
        [0.0031052,0.00061674,0.13053824,-0.00056778],
        [0.00401464,-0.00185226,0.02589161,0.04717514],
        # [0.00106182,0.00516865,0.00065834,0.00308969],        # ! RIP 
        # [0.00024055,0.00868481,0.05629053,0.03818639],
        # [1.0743942e-02,1.5491766e-02,8.5700529e-05,4.0598746e-02],
        # [0.00401647,0.00077669,0.0169516,0.02113935],
        # [-0.00070253,0.00170065,0.08050004,0.00734635],
        # [0.00186555,-0.00151982,0.01879334,-0.00053854],
        # [6.2120578e-04,-4.7482499e-05,3.1835414e-02,1.2831937e-02],
        # [-0.0001491,0.0014499,0.04094484,0.00204615],
        # [0.0064459,0.00222402,0.00206173,0.00210225],
        # [0.00898901,0.07667418,0.01548219,0.03554268],
        # [-1.5523539e-06,2.1837330e-04,2.6588362e-02,1.1840663e-02],
        # [-0.00013785,-0.00078426,0.00273037,0.00276369],
        # [0.01597049,0.04521541,0.00568947,0.00091762],
        # [0.00363583,0.00624786,0.02112816,0.02711718],
        # [0.00104465,0.01759246,0.08661902,0.06389884],
        # [0.00248643,0.00131474,0.05095134,0.05728835],
        # [-0.00050873,-0.00048188,0.01878684,0.00066375],
        # [-0.00039781,0.00099844,0.00364636,0.02317777],
        # [0.00122425,-0.00046588,0.02636435,0.01408367],
        # [0.00338644,0.00031897,0.01394583,0.06442267],
        # [0.00075682,0.00140428,0.05191123,0.0923482,],
        # [0.00047388,0.03364422,0.00252659,0.00224054]
    ]
    print(f'data points: {len(data)}')
    for i, d in enumerate(data):
        ss.insert(d, f"{i}.jpg")
    ss.print()

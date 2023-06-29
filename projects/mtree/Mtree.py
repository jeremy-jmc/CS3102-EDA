import random
import heapq
import numpy as np
import inspect

D = None
M = None


class MNode:
    def __init__(self,
                 pivot: tuple,
                 parent: 'MNode' = None,
                 radius: float = 0,
                 distance_to_parent: float = 0,
                 children: list = None,
                 is_leaf: bool = False):
        self.pivot = pivot
        self.radius = radius
        self.parent = parent
        self.distance_to_parent = distance_to_parent
        self.children = children if children else []
        self.is_leaf = is_leaf

    def compute_radius(self, level: int = 1) -> None:
        dist = 0
        if self.is_leaf:
            dist = max([D(child, self.pivot) for child in self.children])
        else:
            dist = max([child.distance_to_parent +
                       child.radius for child in self.children])
        self.radius = dist
        # print("\t" * level, f'({self.pivot[0]:.2f} {self.pivot[1]:.2f}) -> {self.radius:.2f}\tL: {self.is_leaf}')

    def update(self, level: int = 1) -> None:
        if self.parent:
            self.distance_to_parent = D(self.pivot, self.parent.pivot)
        self.compute_radius(level)

    def find_closest_child(self, dato: list) -> 'MNode':
        min_dist = float('inf')
        closest_child = None
        for child in self.children:
            dist = D(child.pivot, dato)
            if dist < min_dist:
                min_dist = dist
                closest_child = child
        return closest_child

    def get_new_pivots(self) -> tuple:
        """Retorna dos pivotes aleatorios a partir de muestreo 10% y minimizacion de suma de radios"""

        best_radius_sum = float('inf')
        p1, p2 = None, None
        sample: list[list | MNode] = random.sample(
            self.children, max(int(len(self.children) * 0.1), 2))

        if not self.is_leaf:
            sample: list[list] = [s.pivot for s in sample]

        n = len(sample)

        for i in range(n):
            for j in range(i+1, n):
                # for all candidate pairs in samples
                c1, c2 = sample[i], sample[j]
                if c1 == c2:      # ? se puede dar el caso?
                    continue

                rp1 = rp2 = 0

                for point in sample:
                    if point == c1 or point == c2:
                        continue

                    d1 = D(point, c1)
                    d2 = D(point, c2)

                    # hiperplano generalizado
                    if d1 < d2:
                        rp1 = max(rp1, d1)
                    else:
                        rp2 = max(rp2, d2)

                if best_radius_sum > rp1 + rp2:
                    best_radius_sum = rp1 + rp2
                    p1, p2 = c1, c2

        return p1, p2

    def split(self, level: int = 1) -> tuple:
        # print(inspect.currentframe().f_code.co_name)

        p1, p2 = self.get_new_pivots()
        # print('\t' * level, f"p1: {p1}, p2: {p2}")
        new_child_1 = new_child_2 = None
        new_pivot = self.parent.pivot if self.parent else None

        if self.is_leaf:
            new_child_1 = MNode(pivot=p1, parent=self.parent, radius=0,
                                distance_to_parent=0, children=[], is_leaf=True)
            new_child_2 = MNode(pivot=p2, parent=self.parent, radius=0,
                                distance_to_parent=0, children=[], is_leaf=True)

            for child in self.children:
                if D(child, p1) < D(child, p2):
                    new_child_1.children.append(child)
                else:
                    new_child_2.children.append(child)
        else:
            new_child_1 = MNode(pivot=p1, parent=self.parent, radius=0,
                                distance_to_parent=0, children=[], is_leaf=False)
            new_child_2 = MNode(pivot=p2, parent=self.parent, radius=0,
                                distance_to_parent=0, children=[], is_leaf=False)

            for child_node in self.children:
                if D(child_node.pivot, p1) + child_node.radius < D(child_node.pivot, p2) + child_node.radius:
                    child_node.parent = new_child_1
                    child_node.update(level)
                    new_child_1.children.append(child_node)
                else:
                    child_node.parent = new_child_2
                    child_node.update(level)
                    new_child_2.children.append(child_node)

        new_child_1.update(level)
        new_child_2.update(level)

        if new_pivot is None:
            max_radius = -float('inf')
            for c in self.children:
                if self.is_leaf:
                    current_radius = max([D(c, child)
                                         for child in self.children])
                else:
                    current_radius = max(
                        [D(c.pivot, child.pivot) + child.radius for child in self.children if c != child])
                if current_radius > max_radius:
                    max_radius = current_radius
                    new_pivot = c if self.is_leaf else c.pivot

        return new_pivot, new_child_1, new_child_2

    def insert(self, dato, level: int = 1) -> tuple:
        if self.is_leaf:
            if np.any(self.children == dato):
                return (None, None, None)

            self.children.append(dato)
            self.update(level)

            if len(self.children) <= M:
                return (None, None, None)
        else:
            closest_child = self.find_closest_child(dato)
            (new_pivot, new_child_1, new_child_2) = closest_child.insert(dato, level+1)

            if new_child_1 is not None and new_child_2 is not None:
                self.pivot = new_pivot
                idx_to_remove = self.children.index(closest_child)
                self.children.pop(idx_to_remove)

                self.children.append(new_child_1)
                self.children.append(new_child_2)

                self.update(level)

                if len(self.children) <= M:
                    return (None, None, None)
            else:
                self.update(level)
                return (None, None, None)
        return self.split()

    def print(self, rt: bool = False) -> str | None:
        msj = f"pivot: ({self.pivot})\tr: {self.radius:.2f}\tdtp: {self.distance_to_parent}\tc: {len(self.children)} l: {self.is_leaf}"
        if rt:
            return msj
        print(msj)


def choose_node(pq: list[tuple], query : tuple = None) -> MNode:
    pq = sorted(pq, key=lambda x: x[1])
    next_node, dist = pq[0]
    pq.pop(0)
    return next_node

class MTree:
    def __init__(self, disFun, max_size):
        self.root = None
        self.d = disFun             # Funcion de distancia
        self.max_size = max_size    # Capacidad del nodo

        global M, D
        M = max_size                # Alias para max_size
        D = disFun                  # Alias para disFun

    def insert(self, dato: list):
        if not self.root:
            self.root = MNode(pivot=dato, parent=None, radius=0,
                              distance_to_parent=0, children=[dato], is_leaf=True)
        else:
            (new_pivot, new_child_1, new_child_2) = self.root.insert(dato)
            if new_child_1 is not None and new_child_2 is not None:
                self.root = MNode(pivot=new_pivot, parent=None, radius=0, distance_to_parent=0, children=[
                                  new_child_1, new_child_2], is_leaf=False)
                new_child_1.parent = self.root
                new_child_2.parent = self.root
                self.root.update()
                self.root.compute_radius()

    # =========================================================================
    def range_query(self, query, radio, node=None):
        if not node:
            node = self.root
        result = []
        for child in node.children:
            self._range_search(child, query, radio, result)
        
        return result

    def _range_search(self, node: MNode, query: list, radio: float, result: list) -> None:
        """
        Op = node pivot
        Or = child pivot where N is not leaf
        Oj = 
        """
        if not node.is_leaf:
            for child in node.children:
                if abs(D(node.parent.pivot, query) - D(child.pivot, node.parent.pivot)) <= radio + child.radius:
                    if D(child.pivot, query) <= radio + child.radius:
                        self._range_search(child, query, radio, result)
        else:
            for child in node.children:
                if abs(D(node.parent.pivot, query) - D(child, node.parent.pivot)) <= radio:
                    if D(child, query) <= radio:
                        result.append(child)

    def kNN(self, query: list, k: int) -> list:
        result = [(float('inf'), "") for i in range(k)]
        def dfs(node: MNode, q: list):
            nonlocal result, k 
            if node.is_leaf: 
                for child in node.children: 
                    if abs(D(node.parent.pivot, query) - D(child, node.parent.pivot)) <= result[k-1][0] :
                        dist_query = D(child, query)
                        if dist_query < result[k-1][0]:
                            for _ in range(len(result)): 
                                if result[_][0] > dist_query: 
                                    result[_] = (dist_query, child)
                                    break
            else:                 
                for subnode in node.children: 
                    if abs(D(node.parent.pivot, query) - D(subnode.pivot, node.parent.pivot)) <= result[k-1][0] + subnode.radius:
                        if D(subnode.pivot, query) < result[k-1][0]:
                            dfs(subnode, q)
        
        for subnode in self.root.children:
            dfs(subnode, query)
        return  result

    # =========================================================================

    def compute_radius(self, node: MNode = None) -> float:
        dist = 0
        if node.is_leaf:
            dist = max([self.d(child, node.pivot) for child in node.children])
        else:
            dist = max([child.distance_to_parent +
                       child.radius for child in node.children])
        return dist

    def print(self, node: MNode = None, level=0):
        """Imprime el arbol en la consola."""
        if node is None:
            node = self.root
        print(' ' * level * 4,
              f"({node.pivot[0]:.2f}, {node.pivot[1]:.2f})\tn: {len(node.children)}\tdtp: {node.distance_to_parent:.2f}\tr: {node.radius:.2f}")         # \tl: {node.is_leaf}
        if not node.is_leaf:
            for child in node.children:
                self.print(child, level + 1)

    def checkTree(self, node: MNode = None, path: list = None):
        """Verifica que el arbol sea correcto. Retorna True si el arbol es correcto, False en caso contrario.

        Args:
            node (MNode): Nodo raiz de subarbol a verificar. Si no se especifica, se verifica el arbol completo.
            path (list): Lista de pivotes desde la raiz hasta el nodo actual. Se usa para imprimir mensajes de error.
        """

        if node is None:
            node = self.root
            path = []

        
        path.append(node.pivot)

        eps = 0
        errors = []

        if node.parent:
            if abs(self.d(node.pivot, node.parent.pivot) - node.distance_to_parent) > eps:
                errors.append(
                    f"Node {node.pivot} has incorrect distance to parent {node.parent.pivot}")

        if node.is_leaf:
            for child in node.children:
                if self.d(child, node.pivot) > node.radius + eps:
                    errors.append(
                        f"Leaf {child} in node {node.pivot} is outside of its radius")
        else:
            for child in node.children:
                if not self.checkTree(child, path):
                    errors.append(
                        f"Node {child.pivot} has a parent {child.parent.pivot} which does not match expected parent {node.pivot} {self.compute_radius(node)}")

            if len(node.children) > self.max_size:
                errors.append(
                    f"Node {node.pivot} has too many children: {len(node.children)}, max_size is {self.max_size}")

        if abs(self.compute_radius(node) - node.radius) > eps:
            errors.append(
                f"Node {node.pivot} has incorrect radius {node.radius:.2f} (expected {self.compute_radius(node):.2f})")

        path.pop()

        if errors:
            print(f"Errors found in path {'->'.join(str(p) for p in path)}:")
            for error in errors:
                print(f"- {error}")

        return not bool(errors)     # True si no hay errores

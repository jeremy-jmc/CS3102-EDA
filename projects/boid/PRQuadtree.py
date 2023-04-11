import numpy as np
from Bounds import Bounds


"""
    N
O   mid   E
    S
"""

class PRNode:
    def __init__(self, bounds, capacity=4):
        self.bounds = bounds
        self.capacity = capacity
        self.points = []
        self.children = {}
        self.is_divided = False

    def subdivide(self):
        if self.is_divided:
            return

        min_x, max_x = self.bounds.min_x, self.bounds.max_x
        min_y, max_y = self.bounds.min_y, self.bounds.max_y
        mid_x = (min_x + max_x) / 2
        mid_y = (min_y + max_y) / 2

        # * min_x, min_y, max_x, max_y
        nw_bounds = Bounds(min_x, mid_y, mid_x, max_y)
        ne_bounds = Bounds(mid_x, mid_y, max_x, max_y)
        sw_bounds = Bounds(min_x, min_y, mid_x, mid_y)
        se_bounds = Bounds(mid_x, min_y, max_x, mid_y)

        self.children['NW'] = PRNode(nw_bounds)
        self.children['NE'] = PRNode(ne_bounds)
        self.children['SW'] = PRNode(sw_bounds)
        self.children['SE'] = PRNode(se_bounds)
        
        # * redistribuir los puntos
        for point in self.points:
            for child in self.children.values():
                if child.bounds.contains(point.position):
                    child.insert(point)
                    break
            self.points = []
        
        self.is_divided = True

    def is_leaf(self):
        return not self.is_divided
    
    def insert(self, point):
        if self.is_leaf():
            self.points.append(point)

            if len(self.points) > self.capacity:
                self.subdivide()
        else:
            # * find the child to insert the point
            for child in self.children.values():
                if child.bounds.contains(point.position):
                    child.insert(point)
                    break

    def query_circle(self, point, radius, found_points):
        if not self.bounds.intersects_circle(point, radius):
            return
        
        if self.is_leaf():
            for p in self.points:
                if (np.linalg.norm(p.position - point) <= radius):
                    found_points.append(p)
        else:
            for child in self.children.values():
                child.query_circle(point, radius, found_points)

    def nodes(self):
        all_nodes = [self]
        if not self.is_leaf():
            for child in self.children.values():
                all_nodes.extend(child.nodes())
        return all_nodes


class PRQuadtree:
    def __init__(self, bounds, capacity=4):
        self.root = PRNode(bounds, capacity)

    def insert(self, obj, point):
        obj.position = point
        self.root.insert(obj)

    def nodes(self):
        return self.root.nodes()

    def query_circle(self, point, radius):
        found_points = []
        self.root.query_circle(point, radius, found_points)
        return found_points
    
    def clear(self):
        self.root = PRNode(self.root.bounds, self.root.capacity)
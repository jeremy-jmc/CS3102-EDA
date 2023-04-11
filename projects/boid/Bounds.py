import numpy as np

class Bounds:
    def __init__(self, min_x, min_y, max_x, max_y):
        self.min_x = min_x
        self.min_y = min_y
        self.max_x = max_x
        self.max_y = max_y

    @property
    def width(self):
        return self.max_x - self.min_x

    @property
    def height(self):
        return self.max_y - self.min_y

    @property
    def center(self):
        return np.array([(self.min_x + self.max_x) / 2, (self.min_y + self.max_y) / 2])

    def contains(self, point):
        x, y = point
        return self.min_x <= x <= self.max_x and self.min_y <= y <= self.max_y

    def intersects_circle(self, center, radius):
        closest_x = np.clip(center[0], self.min_x, self.max_x)
        closest_y = np.clip(center[1], self.min_y, self.max_y)
        distance_squared = (closest_x - center[0]) ** 2 + (closest_y - center[1]) ** 2
        return distance_squared <= radius ** 2

    def subdivide(self, index):
        if index == 0:
            return Bounds(self.min_x, self.min_y, self.center[0], self.center[1])
        elif index == 1:
            return Bounds(self.center[0], self.min_y, self.max_x, self.center[1])
        elif index == 2:
            return Bounds(self.min_x, self.center[1], self.center[0], self.max_y)
        elif index == 3:
            return Bounds(self.center[0], self.center[1], self.max_x, self.max_y)
        else:
            raise IndexError("Eso no debería ser")

    def subdivisions(self):
        return [
            self.subdivide(0),
            self.subdivide(1),
            self.subdivide(2),
            self.subdivide(3),
        ]

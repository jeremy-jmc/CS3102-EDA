import numpy as np
from PRQuadtree import PRQuadtree
from Bounds import Bounds

class Boid:
    def __init__(self, position, velocity, max_speed, max_force, separation_distance, alignment_distance, cohesion_distance):
        self.position = position
        self.velocity = velocity
        self.max_speed = max_speed
        self.max_force = max_force
        self.separation_distance = separation_distance
        self. alignment_distance =  alignment_distance
        self.  cohesion_distance =   cohesion_distance

    # neighbors: todos los puntos vecinos, o candidatos a vecinos
    def separation(self, neighbors):
        fuerza = np.zeros(2)
        # Calcule con la ecuacion Fs
        for neighbor in neighbors:
            denom = np.linalg.norm(self.position - neighbor.position)
            if denom != 0:
                fuerza += (self.position - neighbor.position) / denom
        return self._limit_force(fuerza)

    # neighbors: todos los puntos vecinos, o candidatos a vecinos
    def alignment(self, neighbors):
        fuerza = np.zeros(2)
        # Calcule con la ecuacion Fa
        for neighbor in neighbors:
            fuerza += neighbor.velocity / len(neighbors)
        fuerza -= self.velocity
        fuerza = self._limit_force(fuerza)
        return fuerza

    # neighbors: todos los puntos vecinos, o candidatos a vecinos
    def cohesion(self, neighbors):
        fuerza = np.zeros(2)
        # Calcule con la ecuacion Fc
        for neighbor in neighbors:
            fuerza += neighbor.position / len(neighbors)
        fuerza -= self.position
        fuerza = self._limit_force(fuerza)
        return fuerza

    def update(self, separation_force, alignment_force, cohesion_force, bounds):
        self.velocity += separation_force + alignment_force + cohesion_force
        self.velocity  = self._limit_speed(self.velocity)
        self.position += self.velocity
        self.bounce(bounds)

    def bounce(self, bounds):
        min_x, min_y, max_x, max_y = bounds.min_x, bounds.min_y, bounds.max_x, bounds.max_y

        if self.position[0] <= min_x or self.position[0] >= max_x:
            self.velocity[0] = -2*self.velocity[0]
            self.position[0] = np.clip(self.position[0], min_x, max_x)

        if self.position[1] <= min_y or self.position[1] >= max_y:
            self.velocity[1] = -2*self.velocity[1]
            self.position[1] = np.clip(self.position[1], min_y, max_y)

    def _limit_force(self, force):
        force_norm = np.linalg.norm(force)
        if force_norm > self.max_force:
            force = (force / force_norm) * self.max_force
        return force

    def _limit_speed(self, velocity):
        speed = np.linalg.norm(velocity)
        if speed > self.max_speed:
            velocity = (velocity / speed) * self.max_speed
        return velocity


class BoidSimulation:
    def __init__(self, boids, bounds, search_radius, quadtree_capacity):
        self.boids = boids
        self.bounds = bounds
        self.search_radius = search_radius
        self.quadtree_capacity = quadtree_capacity

        self.quadtree = PRQuadtree(self.bounds, capacity=self.quadtree_capacity)

    def step(self):
        self.quadtree.clear()
        for boid in self.boids:
            self.quadtree.insert(boid, boid.position)

        for boid in self.boids:
            neighbors = self.quadtree.query_circle(boid.position, self.search_radius)
            separation_force = boid.separation(neighbors)
            alignment_force  = boid.alignment (neighbors)
            cohesion_force   = boid.cohesion  (neighbors)
            boid.update(separation_force, alignment_force, cohesion_force, self.bounds)


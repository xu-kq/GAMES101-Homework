#include <iostream>
#include <vector>

#include "CGL/vector2D.h"

#include "mass.h"
#include "rope.h"
#include "spring.h"

namespace CGL {

    Rope::Rope(Vector2D start, Vector2D end, int num_nodes, float node_mass, float k, vector<int> pinned_nodes)
        : masses(num_nodes), springs(num_nodes - 1)
    {
        // TODO (Part 1): Create a rope starting at `start`, ending at `end`, and containing `num_nodes` nodes.
        Vector2D incr = (end - start) / (num_nodes - 1);
        masses[0] = new Mass(start, node_mass, false);
        Vector2D pos = start + incr;

        for(int i = 1; i < num_nodes; ++i, pos += incr) {
            masses[i] = new Mass(pos, node_mass, false);
            springs[i - 1] = new Spring(masses[i - 1], masses[i], k);
        }

//        Comment-in this part when you implement the constructor
       for (auto &i : pinned_nodes) {
           masses[i]->pinned = true;
       }
    }

    void Rope::simulateEuler(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 2): Use Hooke's law to calculate the force on a node
            Vector2D force, a, b;
            float  vnorm, k, l;

            k = s->k;
            l = s->rest_length;
            a = s->m1->position;
            b = s->m2->position;

            vnorm = (a-b).norm();
            force = k * (b-a)/vnorm*(vnorm - l);
            
            // if(!s->m1->pinned)
            s->m1->forces += force;
            // if(!s->m2->pinned)
            s->m2->forces -= force; 
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                // TODO (Part 2): Add the force due to gravity, then compute the new velocity and position
                double k_d = 1e-2;
                m->forces += gravity * m->mass - k_d * m->velocity;
                Vector2D a;
                a = m->forces / m->mass;
                m->velocity = m->velocity + a * delta_t;
                // semi-implicit Euler
                m->position = m->position + m->velocity * delta_t;

                // TODO (Part 2): Add global damping

            }
            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }

    void Rope::simulateVerlet(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet （solving constraints)
            Vector2D force, a, b;
            float  vnorm, k, l;

            k = s->k;
            l = s->rest_length;
            a = s->m1->position;
            b = s->m2->position;

            vnorm = (a-b).norm();
            force = k * (b-a)/vnorm*(vnorm - l);
            
            // if(!s->m1->pinned)
            s->m1->forces += force;
            // if(!s->m2->pinned)
            s->m2->forces -= force; 
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                Vector2D temp_position = m->position;
                // TODO (Part 3.1): Set the new position of the rope mass
                double k_d = 1e-2;
                m->forces += gravity * m->mass - k_d * m->velocity;
                Vector2D a;
                a = m->forces / m->mass;
                // m->velocity = m->velocity + a * delta_t;
                // m->position = m->position + m->velocity * delta_t;
                m->position = m->position + (1 - 5e-5) * (m->position - m->last_position) + a * delta_t * delta_t;
                m->last_position = temp_position;
                // m->velocity = 1/2/delta_t * (m->position - temp_position);
                // TODO (Part 4): Add global Verlet damping
            }
            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }

        for (auto &s : springs)
        {
            // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet （solving constraints)
            Vector2D a, b, dir;
            double delta_l;
            
            a = s->m1->position;
            b = s->m2->position;

            dir = (b-a).unit();
            delta_l = (a-b).norm() - s->rest_length;
            
            if(s->m1->pinned) {
                s->m2->position -= delta_l * dir;
            } else if(s->m2->pinned) {
                s->m1->position += delta_l * dir;
            } else {
                s->m2->position -= delta_l * dir / 2;
                s->m1->position += delta_l * dir / 2;
            }
        }

    }
}

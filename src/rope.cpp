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
            
            // std::cout << force << std::endl;
            // getchar();
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
                m->forces += gravity * m->mass;
                Vector2D a;
                a = m->forces / m->mass;
                m->velocity = m->velocity + a * delta_t;
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
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                Vector2D temp_position = m->position;
                // TODO (Part 3.1): Set the new position of the rope mass
                
                // TODO (Part 4): Add global Verlet damping
            }
        }
    }
}

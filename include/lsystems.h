#pragma once
#ifndef LSYSTEMS_H
#define LSYSTEMS_H
#include <vector>
#include <unordered_map>
#include <string>
#include <array>

namespace lsystems {


    struct Node {
        float stiffness;
        float gamma;
        float angle;
        std::vector<int> children;
    };

    struct VertexPair {
        std::array<float, 2> start;
        std::array<float, 2> end;
        int node_idx;
    };

    class VerletIntegrator;

    class LSystemPhysics {
        public:
            std::unordered_map<char, std::string> axiom_rules;
            float angleIncrement;
            float length;
            std::string llsystem_string;
            std::vector<Node> tree;
            VerletIntegrator* integrator;

            LSystemPhysics(const std::unordered_map<char, std::string>& rules, float angleInc, float len);
            ~LSystemPhysics();

            void initialize(const std::string& axiom, int iterations, int verbose = 0);

            void perturb_tree();

            std::vector<VertexPair> generate_vertices_pairs(const std::array<float, 2>& start_pos, float start_angle_deg);

            std::vector<float> calculate_external_acceleration(
                const std::array<float, 2>& force_center,
                const std::vector<VertexPair>& vertices_pairs,
                float distance_threshold = 100.0f,
                float force_strength = 0.1f
            );

            std::vector<VertexPair> step(
                const std::array<float, 2>& mouse_pos,
                float dt = 0.1f,
                float distance_threshold = 90.0f,
                float force_strength = 2.0f
            );
        };

}


#endif
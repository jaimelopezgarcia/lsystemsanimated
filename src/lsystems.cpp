
#include <cmath>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <array>
#include <algorithm>
#include "lsystems.h"

namespace lsystems {





        const std::unordered_map<int, std::unordered_map<std::string, float>>& parameters_depth = {
            {0, {{"stiffness", 1.0f}, {"gamma", 0.3f}}},
            {1, {{"stiffness", 0.8f}, {"gamma", 0.1f}}},
            {2, {{"stiffness", 0.6f}, {"gamma", 0.05f}}},
            {3, {{"stiffness", 0.4f}, {"gamma", 0.03f}}},
            {4, {{"stiffness", 0.3f}, {"gamma", 0.02f}}},
            {5, {{"stiffness", 0.2f}, {"gamma", 0.01f}}},
        };


        std::vector<Node> build_tree_vector(
            const std::string& llsystem_string,
            const std::unordered_map<int, std::unordered_map<std::string, float>>& parameters_depth
        ) {
            std::vector<int> stack; // stack of indices into tree
            Node root_node = {parameters_depth.at(0).at("stiffness"), parameters_depth.at(0).at("gamma"),
                0.0f, {}};
            std::vector<Node> tree = {root_node};
            stack.push_back(0); // root node index

            for (char c : llsystem_string) {
                if (c == '[') {
                    int level = stack.size();
                    if (level >= parameters_depth.size()) {
                        std::cerr << "Error: Level exceeds defined parameters depth. using last" << std::endl;
                        level = parameters_depth.size() - 1; // Use the last defined level if out of bounds     
                        
                    }
                    float stiffness = parameters_depth.at(level).at("stiffness");
                    float gamma = parameters_depth.at(level).at("gamma");
                    int parent_idx = stack.back();
                    int idx = tree.size(); // index of the new node
                    tree[parent_idx].children.push_back(idx);
                    Node new_node = {stiffness, gamma, 0.0f, {}};
                    tree.push_back(new_node);
                    stack.push_back(idx);
                } else if (c == ']') {
                    stack.pop_back();
                }
            }
            return tree;
        };

        class VerletIntegrator {
            public:
            /*
            verlet step without velocities is x_n_plus_1 = 2*x_n- x_n_minus_1 + a_n*dt^2
            On the first step we use an euler integration to get the new tree angles,
            and we store the previous angles 
            so def _update_angle(idx,tree_n_plus_1,tree_n, tree_n_minus_1,float a_external,dt):

            float angle_n = tree_n[idx].angle;
                float angle_n_minus_1 = tree_n_minus_1[idx].angle;
            float a_total = -tree_n[idx].stiffness*angle_n + a_external;
                float angle_n_plus_1 = 2*angle_n - angle_n_minus_1 + a_total*dt*dt;
                tree_n_plus_1[idx].angle = angle_n_plus_1;
                children = tree_n[idx].children;
                for (int child_idx : children) {
                    _update_angle(child_idx, tree_n_plus_1, tree_n, tree_n_minus_1, a_external, dt);
                }

            
            */
        bool first_step = true;
        std::vector< Node> tree_n_plus_1;//we dont really need 3 vectors, but it makes the code cleaner
        std::vector< Node > tree_n;
        std::vector< Node > tree_n_minus_1;

        void reset() {
            first_step = true;
        }

        void _update_angle(int idx, std::vector<Node>& tree_n_plus_1, std::vector<Node>& tree_n,
            std::vector<Node>& tree_n_minus_1, const std::vector<float>& a_external, float dt) {
            
                if (first_step) {
                    // Euler step for the first step
                    float angle_n = tree_n[idx].angle;
                    float a_total = -tree_n[idx].stiffness * angle_n + a_external[idx];
                    float angle_n_plus_1 = angle_n + a_total * dt; // Euler integration
                    tree_n_plus_1[idx].angle = angle_n_plus_1;
                } else {
                    // Verlet step
                    float angle_n = tree_n[idx].angle;
                    float angle_n_minus_1 = tree_n_minus_1[idx].angle;
                    //damping
                    float angular_velocity = (angle_n - angle_n_minus_1) / dt;
                    float a_total = -tree_n[idx].stiffness * angle_n + a_external[idx] - tree_n[idx].gamma * angular_velocity;
                    float angle_n_plus_1 = 2 * angle_n - angle_n_minus_1 + a_total * dt * dt;
                    tree_n_plus_1[idx].angle = angle_n_plus_1;
                }

                for (int child_idx : tree_n[idx].children) {
                    _update_angle(child_idx, tree_n_plus_1, tree_n, tree_n_minus_1, a_external, dt);
                }

                }

            void integrate(std::vector<Node>& tree, float dt, const std::vector<float>& a_external) {
            if (first_step) {
                // Initialize the first step
                tree_n_plus_1 = tree;
                tree_n = tree;
                tree_n_minus_1 = tree;
                first_step = false;
            }

                _update_angle(0, tree_n_plus_1, tree_n, tree_n_minus_1, a_external, dt);

                tree_n_minus_1 = tree_n;
                tree_n = tree_n_plus_1;
                tree = tree_n_plus_1; // Update the original tree with the new angles

        }

        };




        std::string apply_rule(std::string& plant_string, std::unordered_map<char,std::string>& axiom_rules){

            std::string new_string = "";
            for (char c : plant_string) {
                if (axiom_rules.find(c) != axiom_rules.end()) {
                    new_string += axiom_rules[c];
                } else {
                    new_string += c;
                }
            }
            return new_string;
        }
        std::string grow_plant(std::string axion, std::unordered_map<char,std::string>& axiom_rules, int iterations,int verbose) {
            std::string plant_string = axion;

            if (verbose) {
                std::cout << "Initial plant string: " << plant_string << std::endl;
            }
            for (int i = 0; i < iterations; ++i) {
                if (verbose) {
                    std::cout << "Iteration " << i << ": " << plant_string << std::endl;
                }
                plant_string = apply_rule(plant_string, axiom_rules);

            }

            if (verbose) {
                std::cout << "Final plant string after " << iterations << " iterations: " << plant_string << std::endl;
            }

            return plant_string;
        }

        float deg_to_rad(float degrees) {
            return degrees * 3.14159265f / 180.0f;
        };

        float rad_to_deg(float radians) {
            return radians * 180.0f / 3.14159265f;
        };



        std::vector<VertexPair> generate_vertices_pairs(
                    const std::string& lsystem_string,
                    std::vector<Node>& tree,
                    std::array<float, 2> start_pos,
                    float start_angle_deg,
                    float segment_length,
                    float angle_increment_deg
                ) {
                    std::vector<VertexPair> vertices_pairs;


                    struct TurtleState {
                    std::array<float, 2> pos; // Position as an array of floats
                    float angle;
                        };

                    std::vector<TurtleState> stack;
                    TurtleState state = {start_pos, start_angle_deg};
                    int node_idx = 0;
                    state.angle += tree[node_idx].angle; // Apply the initial angle from the root node

                    

                    for (char c : lsystem_string) {
                        if (c == 'F') {
                            float radians = state.angle * 3.14159265f / 180.0f;
                            std::array<float, 2> new_pos = {
                                state.pos[0] + segment_length * std::cos(radians),
                                state.pos[1] + segment_length * std::sin(radians)
                            };

                            VertexPair pair;
                            pair.start = {state.pos[0], state.pos[1]};
                            pair.end = {new_pos[0], new_pos[1]};
                            pair.node_idx = node_idx; // Store the current node index
                            vertices_pairs.push_back(pair);

                            state.pos = new_pos;
                        } else if (c == '+') {
                            state.angle += angle_increment_deg;
                        } else if (c == '-') {
                            state.angle -= angle_increment_deg;
                        } else if (c == '[') {
                            stack.push_back(state);
                            node_idx++;
                            if (node_idx < tree.size()) {
                                state.angle += tree[node_idx].angle; // Apply the angle from the current node
                            } else {
                                std::cerr << "Error: Node index out of bounds." << std::endl;
                            }
                        } else if (c == ']') {
                            if (!stack.empty()) {
                                state = stack.back();
                                stack.pop_back();
                            }
                        }

                    }

                    return vertices_pairs;
        }



        void print_tree(const std::vector<Node>& tree){
            for (size_t i = 0; i < tree.size(); ++i) {
                const Node& node = tree[i];
                std::cout << "Node " << i << ": angle = " << node.angle
                        << ", stiffness = " << node.stiffness
                        << ", gamma = " << node.gamma
                        << ", children = [";
                for (size_t j = 0; j < node.children.size(); ++j) {
                    std::cout << node.children[j];
                    if (j < node.children.size() - 1) {
                        std::cout << ", ";
                    }
                }
                std::cout << "]" << std::endl;
            }
        }


            
                                                        
        float signed_angle(float x1, float y1, float x2, float y2) {
            float dot = x1 * x2 + y1 * y2;
            float det = x1 * y2 - y1 * x2;
            return -std::atan2(det, dot);
        }

        std::vector<float> calculate_external_acceleration(
            std::array<float,2> force_center,
            const std::vector<Node>& tree,
            const std::vector<VertexPair>& vertices_pairs,
            float distance_threshold = 100.0f,
            float force_strength = 0.1f
        ) {

            //just one force per branch, so we keep track of the visited node idex
            std::vector<int> visited_nodes;
            std::vector<float> a_external(tree.size(), 0.0f);
            float distance_threshold_sq = distance_threshold * distance_threshold;
            for (const VertexPair& pair : vertices_pairs) {

                //lets check if the node has already been visited

                float dx = force_center[0] - pair.start[0];
                float dy = force_center[1] - pair.start[1];
                float dist = dx*dx + dy*dy;

                if (dist < distance_threshold_sq) {

                if (std::find(visited_nodes.begin(), visited_nodes.end(), pair.node_idx) != visited_nodes.end()) {
                    continue; // Skip this pair if the node has already been processed
                }
                visited_nodes.push_back(pair.node_idx); // Mark this node as visited
                    float bx = pair.end[0] - pair.start[0];
                    float by = pair.end[1] - pair.start[1];
                    float blen = bx*bx + by*by;
                    if (blen > 1e-6) {
                        bx /= blen;
                        by /= blen;
                    }
                    float mx = dx / dist;
                    float my = dy / dist;
                    float angle = signed_angle(bx, by, mx, my); // radians
                    float torque = force_strength * angle * (1.0f - dist / distance_threshold_sq);
                    a_external[pair.node_idx] += torque;
                }
            }
            return a_external;
        }

    
    LSystemPhysics::LSystemPhysics(const std::unordered_map<char, std::string>& rules, float angleInc, float len)
        : axiom_rules(rules), angleIncrement(angleInc), length(len), integrator(nullptr) {}

    LSystemPhysics::~LSystemPhysics() {
        delete integrator;
    }

    void LSystemPhysics::initialize(const std::string& axiom, int iterations, int verbose) {
        llsystem_string = grow_plant(axiom, axiom_rules, iterations, verbose);
        tree = build_tree_vector(llsystem_string, parameters_depth);
        perturb_tree();
        if (integrator) delete integrator;
        integrator = new VerletIntegrator();
    }

    void LSystemPhysics::perturb_tree() {
        for (Node& node : tree) {
            node.angle += static_cast<float>(rand() % 40 - 20);
        }
    }

    std::vector<VertexPair> LSystemPhysics::generate_vertices_pairs(const std::array<float, 2>& start_pos, float start_angle_deg) {
        return lsystems::generate_vertices_pairs(llsystem_string, tree, start_pos, start_angle_deg, length, angleIncrement);
    }

    std::vector<float> LSystemPhysics::calculate_external_acceleration(
        const std::array<float, 2>& force_center,
        const std::vector<VertexPair>& vertices_pairs,
        float distance_threshold,
        float force_strength
    ) {
        return lsystems::calculate_external_acceleration(force_center, tree, vertices_pairs, distance_threshold, force_strength);
    }

    std::vector<VertexPair> LSystemPhysics::step(
        const std::array<float, 2>& mouse_pos,
        float dt,
        float distance_threshold,
        float force_strength
    ) {
        std::array<float, 2> start_pos = {400.0f, 590.0f};
        float start_angle_deg = -90.0f;
        auto vertices_pairs = generate_vertices_pairs(start_pos, start_angle_deg);
        auto a_external = calculate_external_acceleration(mouse_pos, vertices_pairs, distance_threshold, force_strength);
        integrator->integrate(tree, dt, a_external);
        return generate_vertices_pairs(start_pos, start_angle_deg);
    }

}
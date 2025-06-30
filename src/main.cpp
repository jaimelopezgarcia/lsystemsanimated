//#include <SFML/Graphics.hpp>
#include "lsystems.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <array>
#include <sstream>



/*


void draw_lsystem_basic(
    sf::RenderWindow& window,
    const std::vector<lsystems::VertexPair>& vertices_pairs
) {
    for (const lsystems::VertexPair& pair : vertices_pairs) {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(pair.start[0], pair.start[1]), sf::Color::Green),
            sf::Vertex(sf::Vector2f(pair.end[0], pair.end[1]), sf::Color::Green)
        };
        window.draw(line, 2, sf::Lines);
    }
}

*/
//lets make a function to parse some keys and values chars into an unordered_map
std::unordered_map<char, std::string> parse_rules(const char* keys, const char* values) {
        //keys will be \n separated keys, and values will be \n separated values
            std::unordered_map<char, std::string> rules;
            //now we have to split the keys and values by \n
            std::istringstream key_stream(keys);
            std::istringstream value_stream(values);
            std::string key_line, value_line;
            while (std::getline(key_stream, key_line) && std::getline(value_stream, value_line)) {
                if (!key_line.empty())
                    rules[key_line[0]] = value_line;
            }
        
            //lets make sure all parentheses are balanced for every value
            for (auto& rule : rules) {
                std::string value = rule.second;
                int balance = 0;
                for (char c : value) {
                    if (c == '[') {
                        balance++;
                    } else if (c == ']') {
                        balance--;
                    }
                }
                if (balance != 0) {
                    std::string error_message = "Unbalanced parentheses in key: ";
                    error_message += rule.first;
                    error_message += " with value: ";
                    error_message += value;
                    throw std::runtime_error(error_message);
                }
            }
            return rules;
        }
extern "C" {

    lsystems::LSystemPhysics* lsystem_physics_create(
        const char* keys,
        const char* values,
        const char* axiom,
        int iterations){
        //for now we'll hardcode the rules, angle increment and length
        //std::unordered_map<char, std::string> axiom_rules = {
        //    {'F', "FF"},
        //    {'X', "F[+X]F[-X]+X"}
        //};
        std::unordered_map<char, std::string> axiom_rules = parse_rules(keys, values);
        float angleIncrement = 20.0f; // Angle increment in degrees
        float length = 10.0f; // Length of each segment
        lsystems::LSystemPhysics* lsysptr = new lsystems::LSystemPhysics(axiom_rules, angleIncrement, length);
        lsysptr->initialize(axiom, iterations, 1); // axiom

        return lsysptr;
    
        }

    void lsystem_physics_destroy(lsystems::LSystemPhysics* lsysptr) {
        delete lsysptr;

    }

    //for now we just step without mouse position
    float* lsystem_physics_step(
        lsystems::LSystemPhysics* lsysptr,
        int* out_size,
        int mouseX,
        int mouseY,
        float dt,
        float distance_threshold,
        float force_strength
    ) {
       // std::cout << "Stepping LSystemPhysics with mouse position: (" << mouseX << ", " << mouseY << ")" << std::endl;
        //float dt = 0.1f; // Time step
        //float distance_threshold = 90.0f; // Distance threshold for external force
        //float force_strength = 5.0f; // Strength of the external force
        std::array<float, 2> mouse_pos = {static_cast<float>(mouseX), static_cast<float>(mouseY)};
        auto vertices_pairs = lsysptr->step(mouse_pos, dt, distance_threshold, force_strength);
        
        *out_size = vertices_pairs.size() * 4; // Each VertexPair has 4 floats (start.x, start.y, end.x, end.y)
        float* result = (float*)malloc(*out_size * sizeof(float));
        if (!result) {
            std::cerr << "Memory allocation failed!" << std::endl;
            *out_size = 0;
            return nullptr; // Return nullptr if memory allocation fails
        }
        
        for (size_t i = 0; i < vertices_pairs.size(); ++i) {
            const lsystems::VertexPair& pair = vertices_pairs[i];
            result[i * 4] = pair.start[0];
            result[i * 4 + 1] = pair.start[1];
            result[i * 4 + 2] = pair.end[0];
            result[i * 4 + 3] = pair.end[1];
        }
        
        return result;



    }

    void free_float_buffer(float* buffer) {
        if (buffer) {
            free(buffer);
        }
    }


}
/*
int main() {
    const int windowWidth = 800;
    const int windowHeight = 600;

    // Newline-separated keys and values
    const char* keys = "F\nX";
    const char* values = "FF\nF[+X]F[-X]+X";
    const char* axiom = "X";
    int iterations = 5;
    float dt = 0.1f; // Time step
    float distance_threshold = 90.0f; // Distance threshold for external force
    float force_strength = 2.0f; // Strength of the external force


    // Create the LSystemPhysics object using the C API
    lsystems::LSystemPhysics* lsys = lsystem_physics_create(keys, values, axiom, iterations);

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "L-System Viewer");
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        sf::Vector2i mouse = sf::Mouse::getPosition(window);
        int mouseX = mouse.x;
        int mouseY = mouse.y;

        // Step the simulation using the C API
        int out_size = 0;
        float* vertices_buffer = lsystem_physics_step(lsys, &out_size, mouseX, mouseY, dt, distance_threshold, force_strength);

        // Convert the float buffer to VertexPairs for drawing
        std::vector<lsystems::VertexPair> vertices_pairs;
        for (int i = 0; i + 3 < out_size; i += 4) {
            lsystems::VertexPair pair;
            pair.start[0] = vertices_buffer[i];
            pair.start[1] = vertices_buffer[i + 1];
            pair.end[0] = vertices_buffer[i + 2];
            pair.end[1] = vertices_buffer[i + 3];
            vertices_pairs.push_back(pair);
        }

        window.clear(sf::Color::Black);
        draw_lsystem_basic(window, vertices_pairs);
        window.display();

        // Free the buffer allocated by the C API
        free_float_buffer(vertices_buffer);
    }

    // Destroy the LSystemPhysics object using the C API
    lsystem_physics_destroy(lsys);

    return 0;
}

*/
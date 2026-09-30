#include <algorithm>
#include <cmath>
#include <iostream>
#include <optional>
#include <vector>

#include <SFML/Graphics.hpp>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const int FPS_LIMIT = 30;
const int ANIMATION_FRAMES = 120;
const float PI = 3.14159f;

using Point2D = sf::Vector2f;

// TODO: (Part 1) Define a function that samples a cubic Bezier curve at t in [0, 1].
Point2D getPoint(const std::vector<sf::Vector2f>& pts, float t) {
    float oneMinusT = 1.0f - t;

    return oneMinusT * oneMinusT * oneMinusT * pts[0] + 3.0f * oneMinusT * oneMinusT * t * pts[1] +
           3.0f * oneMinusT * t * t * pts[2] + t * t * t * pts[3];
}

// TODO: (Part 2) Define a function that returns the curve's slope at t in [0, 1].
Point2D getSlope(const std::vector<sf::Vector2f>& pts, float t) {
    float oneMinusT = 1.0f - t;

    return 3.0f * oneMinusT * oneMinusT * (pts[1] - pts[0]) +
           6.0f * oneMinusT * t * (pts[2] - pts[1]) + 3.0f * t * t * (pts[3] - pts[2]);
}

// TODO: (Part 1) Store four control points for the curve.
std::vector<sf::Vector2f> points = {
    {80.0f, 450.0f}, {160.0f, 200.0f}, {280.0f, 200.0f}, {360.0f, 450.0f}};
// TODO: (Part 2) Track animation time for the square moving along the curve.
int animationFrame = 0;
// TODO: (Part 3) Track the index of the control point being dragged.
int selectedPoint = -1;

std::vector<sf::Vector2f> getSegment(int start) {
    return {points[start], points[start + 1], points[start + 2], points[start + 3]};
}

float getDistance(sf::Vector2f a, sf::Vector2f b) {
    float x = a.x - b.x;
    float y = a.y - b.y;
    return std::sqrt(x * x + y * y);
}

void movePoint(int index, sf::Vector2f newPosition) {
    sf::Vector2f movement = newPosition - points[index];

    if (index % 3 == 0) {
        points[index] = newPosition;
        if (index > 0) {
            points[index - 1] += movement;
        }
        if (index + 1 < static_cast<int>(points.size())) {
            points[index + 1] += movement;
        }
        return;
    }

    points[index] = newPosition;

    int anchor = index % 3 == 2 ? index + 1 : index - 1;
    int otherHandle = index % 3 == 2 ? anchor + 1 : anchor - 1;

    if (otherHandle < 0 || otherHandle >= static_cast<int>(points.size())) {
        return;
    }

    float otherLength = getDistance(points[otherHandle], points[anchor]);
    sf::Vector2f direction = points[index] - points[anchor];
    float directionLength = getDistance(points[index], points[anchor]);

    if (directionLength > 0.0f) {
        direction /= directionLength;
        points[otherHandle] = points[anchor] - direction * otherLength;
    }
}

void addSegment() {
    sf::Vector2f lastPoint = points.back();
    sf::Vector2f direction = lastPoint - points[points.size() - 2];
    float directionLength = getDistance(lastPoint, points[points.size() - 2]);

    if (directionLength > 0.0f) {
        direction /= directionLength;
    }

    points.push_back(lastPoint + direction * 100.0f);
    points.push_back(lastPoint + sf::Vector2f{160.0f, 100.0f});
    points.push_back(lastPoint + sf::Vector2f{240.0f, 0.0f});
}

void handleInput(sf::Window& window, bool& shouldQuit) {
    while (const std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
            shouldQuit = true;
        } else if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
            // TODO: (Part 3) On left-click, select the closest control point
            // using mouse->position and start dragging it.
            if (mouse->button == sf::Mouse::Button::Left) {
                sf::Vector2f mousePosition{static_cast<float>(mouse->position.x),
                                           static_cast<float>(mouse->position.y)};
                float closestDistance = WINDOW_WIDTH + WINDOW_HEIGHT;

                for (int i = 0; i < static_cast<int>(points.size()); i++) {
                    float distance = getDistance(mousePosition, points[i]);
                    if (distance < closestDistance) {
                        closestDistance = distance;
                        selectedPoint = i;
                    }
                }
            }
        } else if (const auto* mouse = event->getIf<sf::Event::MouseButtonReleased>()) {
            // TODO: (Part 3) On left-button release, stop dragging.
            if (mouse->button == sf::Mouse::Button::Left) {
                selectedPoint = -1;
            }
        } else if (const auto* mouse = event->getIf<sf::Event::MouseMoved>()) {
            // TODO: (Part 3) Move the selected control point to mouse->position.
            if (selectedPoint != -1) {
                sf::Vector2f mousePosition{static_cast<float>(mouse->position.x),
                                           static_cast<float>(mouse->position.y)};
                movePoint(selectedPoint, mousePosition);
            }
            // TODO: (Part 4) Maintain matching slopes at shared endpoints.
            // When moving point 3, move point 5 without changing its distance
            // from point 4 (point numbers here start at 1).
        } else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            // TODO: (Part 4) '+' adds three control points; '-' removes three,
            // keeping at least four points.
            if (key->code == sf::Keyboard::Key::Add || key->code == sf::Keyboard::Key::Equal) {
                addSegment();
            } else if ((key->code == sf::Keyboard::Key::Subtract ||
                        key->code == sf::Keyboard::Key::Hyphen) &&
                       points.size() > 4) {
                points.resize(points.size() - 3);
                selectedPoint = -1;
            }
        }
    }
}

void render(sf::RenderWindow& window) {
    window.clear(sf::Color::Black);
    // ====== ====== ======
    // TODO: (Part 1) Sample GetPoint over t in [0, 1] and connect samples using the line-drawing
    // code from your project. Draw all four control points as circles after drawing the curve.
    // ====== ====== ======
    sf::VertexArray curve(sf::PrimitiveType::LineStrip);
    for (int start = 0; start + 3 < static_cast<int>(points.size()); start += 3) {
        std::vector<sf::Vector2f> segment = getSegment(start);
        for (int sample = 0; sample <= 100; sample++) {
            float t = sample / 100.0f;
            curve.append(sf::Vertex{getPoint(segment, t), sf::Color::Yellow});
        }
    }
    window.draw(curve);

    // ====== ====== ======
    // TODO: (Part 2) Draw a small square moving repeatedly along the curve.
    // Use GetSlope to orient it to the curve at each time step.
    // ====== ====== ======
    int segmentCount = static_cast<int>((points.size() - 1) / 3);
    float fullTime =
        static_cast<float>(animationFrame % (ANIMATION_FRAMES * segmentCount)) / ANIMATION_FRAMES;
    int currentSegment = std::min(static_cast<int>(fullTime), segmentCount - 1);
    float segmentTime = fullTime - currentSegment;
    std::vector<sf::Vector2f> animatedSegment = getSegment(currentSegment * 3);
    sf::Vector2f squarePosition = getPoint(animatedSegment, segmentTime);
    sf::Vector2f slope = getSlope(animatedSegment, segmentTime);
    float angle = std::atan2(slope.y, slope.x) * 180.0f / PI;

    sf::RectangleShape square({18.0f, 18.0f});
    square.setOrigin({9.0f, 9.0f});
    square.setPosition(squarePosition);
    square.setRotation(sf::degrees(angle));
    square.setFillColor(sf::Color::Cyan);
    window.draw(square);
    animationFrame++;

    // ====== ====== ======
    // TODO: (Part 3) Draw control handles from point 1 to 2 and point 3 to 4.
    // TODO: (Part 4) Draw all connected cubic Bezier segments and their handles.
    // ====== ====== ======
    sf::VertexArray handles(sf::PrimitiveType::Lines);
    for (int start = 0; start + 3 < static_cast<int>(points.size()); start += 3) {
        handles.append(sf::Vertex{points[start], sf::Color(120, 120, 120)});
        handles.append(sf::Vertex{points[start + 1], sf::Color(120, 120, 120)});
        handles.append(sf::Vertex{points[start + 2], sf::Color(120, 120, 120)});
        handles.append(sf::Vertex{points[start + 3], sf::Color(120, 120, 120)});
    }
    window.draw(handles);

    for (int i = 0; i < static_cast<int>(points.size()); i++) {
        sf::CircleShape controlPoint(8.0f);
        controlPoint.setOrigin({8.0f, 8.0f});
        controlPoint.setPosition(points[i]);
        controlPoint.setFillColor(i % 3 == 0 ? sf::Color::Red : sf::Color::Magenta);
        window.draw(controlPoint);
    }

    // ====== ====== ======
    // TODO: (Bonus) Support multiple curves, a Galaga screen overlay at a 1:2 ratio, and exporting
    // curve points as C++ code for Project 1b.
    // ====== ====== ======

    window.display();
}

int main() {
    sf::RenderWindow window;

    try {
        // Initialize window
        window.create(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Bezier Curve Editor");
        window.setFramerateLimit(FPS_LIMIT);
        // Prevent key repeats.
        window.setKeyRepeatEnabled(false);

        bool shouldQuit = false;
        // Main game loop
        while (window.isOpen()) {
            handleInput(window, shouldQuit);
            if (shouldQuit) {
                break;
            }
            render(window);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}

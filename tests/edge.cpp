#include "../source/Vector2D.hpp"
#include "../source/Transform2D.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

static int failures = 0;

static void expect(bool cond, const char* name) {
    if (!cond) {
        std::cerr << "FAIL " << name << "\n";
        ++failures;
    }
}

static int check_allowed_edges() {
    Vector2D tiny{2.0f * EPSILON, 0.0f};
    Vector2D unit = tiny.normalized();
    expect(unit.equals(Vector2D{1.0f, 0.0f}, 1e-5f), "normalize just above EPSILON");

    const float deg30 = 30.0f * 3.14159265358979323846f / 180.0f;
    const float cos30 = std::cos(deg30);
    const float sin30 = std::sin(deg30);
    Transform2D rotation = Transform2D::rotation(deg30);
    Vector2D turned = rotation.transform_point(Vector2D{1.0f, 0.0f});
    expect(turned.equals(Vector2D{cos30, sin30}, 1e-5f), "rotate 30 degrees");

    Vector2D almost = Vector2D{1.0f, 0.0f} / (1.0f + EPSILON);
    // With official EPSILON = 1e-4, |1/(1+EPS) - 1| is about EPSILON itself.
    expect(std::abs(almost.x - 1.0f) <= EPSILON * 2.0f, "divide by scalar just above EPSILON");

    if (failures != 0) {
        std::cerr << failures << " failed\n";
        return 1;
    }
    std::cout << "ok allowed edges\n";
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "uso: edge <allowed|norm0|normalize0|div0|diveps>\n";
        return 2;
    }
    const std::string mode = argv[1];
    if (mode == "allowed") {
        return check_allowed_edges();
    }
    if (mode == "norm0") {
        (void)Vector2D{0.0f, 0.0f}.normalized();
    } else if (mode == "normalize0") {
        Vector2D zero{0.0f, 0.0f};
        zero.normalize();
    } else if (mode == "div0") {
        (void)(Vector2D{1.0f, 2.0f} / 0.0f);
    } else if (mode == "diveps") {
        (void)(Vector2D{1.0f, 2.0f} / EPSILON);
    } else {
        std::cerr << "modo desconhecido\n";
        return 2;
    }
    std::cerr << "pre-condicao nao interrompeu o programa\n";
    return 1;
}

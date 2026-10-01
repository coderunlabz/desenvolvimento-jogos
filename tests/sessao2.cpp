#include "../source/RigidBody2D.hpp"
#include "../source/Collision2D.hpp"
#include "../source/Vector2D.hpp"
#include "../source/Transform2D.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

static int failures = 0;

static void expect(bool cond, const char* name) {
    if (!cond) {
        std::cerr << "FAIL " << name << "\n";
        ++failures;
    }
}

static void test_session1_smoke() {
    Vector2D a{3.0f, 4.0f};
    expect(std::abs(a.length() - 5.0f) < EPSILON, "s1 length");
    expect(a.normalized().equals(Vector2D{0.6f, 0.8f}), "s1 normalized");
    expect((2.0f * a).equals(Vector2D{6.0f, 8.0f}), "s1 left mul");

    Transform2D t = Transform2D::translation(10.0f, -3.0f);
    expect(t.transform_point(Vector2D{1.0f, 2.0f}).equals(Vector2D{11.0f, -1.0f}), "s1 translate");
}

static void test_integrate() {
    RigidBody2D body;
    body.acceleration = Vector2D{2.0f, 0.0f};
    body.integrate(0.5f);
    expect(body.velocity.equals(Vector2D{1.0f, 0.0f}), "integrate vel after 0.5");
    expect(body.position.equals(Vector2D{0.5f, 0.0f}), "integrate pos uses new vel");

    RigidBody2D straight;
    straight.velocity = Vector2D{3.0f, -1.0f};
    straight.integrate(1.0f);
    expect(straight.velocity.equals(Vector2D{3.0f, -1.0f}), "zero accel keeps vel");
    expect(straight.position.equals(Vector2D{3.0f, -1.0f}), "zero accel moves by vel*dt");

    RigidBody2D still;
    still.integrate(0.25f);
    expect(still.position.equals(Vector2D{0.0f, 0.0f}), "all zero stays");
    expect(still.velocity.equals(Vector2D{0.0f, 0.0f}), "all zero vel");

    RigidBody2D down;
    down.acceleration = Vector2D{0.0f, 10.0f};
    down.integrate(1.0f);
    expect(down.velocity.equals(Vector2D{0.0f, 10.0f}), "down vel");
    expect(down.position.equals(Vector2D{0.0f, 10.0f}), "y grows downward");
}

static void test_aabb() {
    AABB a{Vector2D{0.0f, 0.0f}, Vector2D{10.0f, 10.0f}};
    AABB b{Vector2D{10.0f, 0.0f}, Vector2D{20.0f, 10.0f}};
    expect(a.intersects(b), "touching edge overlaps");
    expect(b.intersects(a), "touching edge symmetric");

    AABB inside{Vector2D{2.0f, 2.0f}, Vector2D{4.0f, 4.0f}};
    expect(a.intersects(inside), "contained overlaps");

    AABB far{Vector2D{11.0f, 0.0f}, Vector2D{20.0f, 10.0f}};
    expect(!a.intersects(far), "gap does not overlap");

    AABB above{Vector2D{0.0f, 10.0f}, Vector2D{10.0f, 20.0f}};
    expect(a.intersects(above), "touching on y overlaps");
}

static void test_bounds() {
    Collision2D shape;
    expect(shape.halfExtents.equals(Vector2D{TILE_SIZE * 0.5f, TILE_SIZE * 0.5f}), "default 32x32");

    AABB box = shape.bounds(Vector2D{100.0f, 200.0f});
    expect(box.min.equals(Vector2D{68.0f, 168.0f}), "bounds min");
    expect(box.max.equals(Vector2D{132.0f, 232.0f}), "bounds max");

    Collision2D custom;
    custom.halfExtents = Vector2D{5.0f, 10.0f};
    AABB c = custom.bounds(Vector2D{0.0f, 0.0f});
    expect(c.min.equals(Vector2D{-5.0f, -10.0f}), "custom min");
    expect(c.max.equals(Vector2D{5.0f, 10.0f}), "custom max");
}

int main() {
    test_session1_smoke();
    test_integrate();
    test_aabb();
    test_bounds();

    if (failures != 0) {
        std::cerr << failures << " failed\n";
        return 1;
    }
    std::cout << "ok sessao2\n";
    return 0;
}

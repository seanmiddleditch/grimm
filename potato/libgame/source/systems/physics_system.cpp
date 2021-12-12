// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/game/components/rigidbody_component.h"
#include "potato/game/components/transform_component.h"
#include "potato/game/entity_manager.h"
#include "potato/game/space.h"
#include "potato/game/system.h"

#include <glm/gtx/rotate_vector.hpp>
#include <btBulletDynamicsCommon.h>

namespace up {
    namespace {
        struct PhysicsWorld {
            PhysicsWorld() : dispatcher(&config), world(&dispatcher, &broadphase, &solver, &config) { }

            btRigidBody* addRigidBody(glm::vec3 position, float mass);

            btDefaultCollisionConfiguration config;
            btCollisionDispatcher dispatcher;
            btDbvtBroadphase broadphase;
            btSequentialImpulseConstraintSolver solver;
            btDiscreteDynamicsWorld world;
            btRigidBody* ground = nullptr;
            float tickRate = 1.f / 60.f;
        };

        class RigidBodyObserver final : public ComponentObserver<RigidBodyComponent> {
        public:
            RigidBodyObserver(EntityManager& entities, PhysicsWorld& world) : _entities(entities), _world(world) { }

            void onAdd(EntityId entityId, RigidBodyComponent& body) override;
            void onRemove(EntityId entityId, RigidBodyComponent& body) override;

        private:
            EntityManager& _entities;
            PhysicsWorld& _world;
        };

        struct BulletBody {
            btRigidBody* body = nullptr;
        };

        class PhysicsSystem final : public System {
        public:
            explicit PhysicsSystem(Space& space) : System(space), _bodyObserver(space.entities(), _world) { }

            void update(float) override;
            void start() override;
            void stop() override;

        private:
            PhysicsWorld _world;
            RigidBodyObserver _bodyObserver;
        };
    } // namespace

    void registerPhysicsSystem(Space& space) { space.addSystem<PhysicsSystem>(); }

    btRigidBody* PhysicsWorld::addRigidBody(glm::vec3 position, float mass) {
        static btBoxShape cube({0.5f, 0.5f, 0.5f});

        btVector3 localInertia(0.f, 0.f, 0.f);
        cube.calculateLocalInertia(mass, localInertia);

        box<btRigidBody> bulletBody;
        bulletBody.reset(new btRigidBody(mass, nullptr, &cube, localInertia));

        btTransform& worldTrans = bulletBody->getWorldTransform();
        worldTrans.setOrigin({position.x, position.y, position.z});

        // test impulse
        bulletBody->applyImpulse({0.f, 5.f, 2.f}, {0.4f, 0.4f, 0.2f});

        world.addRigidBody(bulletBody.get());

        return bulletBody.release();
    }

    void PhysicsSystem::update(float deltaTime) {
        _world.world.stepSimulation(deltaTime, 12, _world.tickRate);

        // Apply physics motion to transforms
        //  TODO: be smarter/faster about this (btMotionState?)
        space().entities().select<TransformComponent, RigidBodyComponent, BulletBody const>(
            [&](EntityId, TransformComponent& trans, RigidBodyComponent& body, BulletBody const& bulletBody) {
                UP_GUARD_VOID(bulletBody.body != nullptr);

                btTransform const& worldTrans = bulletBody.body->getWorldTransform();
                btVector3 const& origin = worldTrans.getOrigin();
                trans.position = {origin.x(), origin.y(), origin.z()};

                btQuaternion const& rot = worldTrans.getRotation();
                trans.rotation = {rot.x(), rot.y(), rot.z(), rot.w()};
            });
    }

    void PhysicsSystem::start() {
        static btStaticPlaneShape groundPlane({0.f, 1.f, 0.f}, 1.f);
        _world.ground = new btRigidBody(0.f, nullptr, &groundPlane);
        _world.world.addRigidBody(_world.ground);

        space().entities().registerComponent<BulletBody>();
        space().entities().observe(_bodyObserver);

        space().entities().select<RigidBodyComponent>([&](EntityId entityId, RigidBodyComponent& body) {
            auto* const transform = space().entities().getComponentSlow<TransformComponent>(entityId);
            glm::vec3 position = transform != nullptr ? transform->position : glm::vec3{0.f, 0.f, 0.f};

            auto& bulletBody = space().entities().addComponent<BulletBody>(entityId);
            bulletBody.body = _world.addRigidBody(position, body.mass);
        });
    }

    void PhysicsSystem::stop() {
        space().entities().unobserve(_bodyObserver);

        _world.world.removeRigidBody(_world.ground);
        delete _world.ground;
        _world.ground = nullptr;
    }

    void RigidBodyObserver::onAdd(EntityId entityId, RigidBodyComponent& body) {
        auto* const transform = _entities.getComponentSlow<TransformComponent>(entityId);
        glm::vec3 position = transform != nullptr ? transform->position : glm::vec3{0.f, 0.f, 0.f};

        auto& bulletBody = _entities.addComponent<BulletBody>(entityId);
        bulletBody.body = _world.addRigidBody(position, body.mass);
    }

    void RigidBodyObserver::onRemove(EntityId entityId, RigidBodyComponent& body) {
        auto* const bulletBody = _entities.getComponentSlow<BulletBody>(entityId);
        if (bulletBody == nullptr) {
            return;
        }

        _world.world.removeRigidBody(bulletBody->body);
        delete bulletBody->body;

        _entities.removeComponent<BulletBody>(entityId);
    }
} // namespace up

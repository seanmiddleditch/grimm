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

            btRigidBody* addRigidBody(glm::vec3 position, glm::vec3 linearVelocity);

            btDefaultCollisionConfiguration config;
            btCollisionDispatcher dispatcher;
            btDbvtBroadphase broadphase;
            btSequentialImpulseConstraintSolver solver;
            btDiscreteDynamicsWorld world;
            float tickRate = 1.f / 60.f;
        };

        class RigidBodyObserver final : public ComponentObserver<component::RigidBody> {
        public:
            RigidBodyObserver(EntityManager& entities, PhysicsWorld& world) : _entities(entities), _world(world) { }

            void onAdd(EntityId entityId, component::RigidBody& body) override;
            void onRemove(EntityId entityId, component::RigidBody& body) override;

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

    btRigidBody* PhysicsWorld::addRigidBody(glm::vec3 position, glm::vec3 linearVelocity) {
        static btBoxShape cube({0.5f, 0.5f, 0.5f});
        box<btRigidBody> bulletBody;
        bulletBody.reset(new btRigidBody(1.f, nullptr, &cube));

        btTransform& worldTrans = bulletBody->getWorldTransform();
        worldTrans.setOrigin({position.x, position.y, position.z});

        bulletBody->setLinearVelocity({linearVelocity.x, linearVelocity.y, linearVelocity.z});

        world.addRigidBody(bulletBody.get());

        return bulletBody.release();
    }

    void PhysicsSystem::update(float deltaTime) {
        _world.world.stepSimulation(deltaTime, 12, _world.tickRate);

        // Apply physics motion to transforms
        //  TODO: be smarter/faster about this (btMotionState?), update rotation
        space().entities().select<component::Transform, component::RigidBody, BulletBody const>(
            [&](EntityId, component::Transform& trans, component::RigidBody& body, BulletBody const& bulletBody) {
                UP_GUARD_VOID(bulletBody.body != nullptr);

                btTransform const& worldTrans = bulletBody.body->getWorldTransform();
                btVector3 const& origin = worldTrans.getOrigin();
                trans.position = {origin.x(), origin.y(), origin.z()};

                btVector3 const& linearVelocity = bulletBody.body->getLinearVelocity();
                body.linearVelocity = {linearVelocity.x(), linearVelocity.y(), linearVelocity.y()};

                if (trans.position.y <= 0.f) {
                    trans.position.y = 0.f;
                    bulletBody.body->setLinearVelocity({0, 0, 0});
                }
            });
    }

    void PhysicsSystem::start() {
        space().entities().registerComponent<BulletBody>();
        space().entities().observe(_bodyObserver);

        space().entities().select<component::RigidBody>([&](EntityId entityId, component::RigidBody& body) {
            auto* const transform = space().entities().getComponentSlow<component::Transform>(entityId);
            glm::vec3 position = transform != nullptr ? transform->position : glm::vec3{0.f, 0.f, 0.f};

            auto& bulletBody = space().entities().addComponent<BulletBody>(entityId);
            bulletBody.body = _world.addRigidBody(position, body.linearVelocity);
        });
    }

    void PhysicsSystem::stop() { space().entities().unobserve(_bodyObserver); }

    void RigidBodyObserver::onAdd(EntityId entityId, component::RigidBody& body) {
        auto* const transform = _entities.getComponentSlow<component::Transform>(entityId);
        glm::vec3 position = transform != nullptr ? transform->position : glm::vec3{0.f, 0.f, 0.f};

        auto& bulletBody = _entities.addComponent<BulletBody>(entityId);
        bulletBody.body = _world.addRigidBody(position, body.linearVelocity);
    }

    void RigidBodyObserver::onRemove(EntityId entityId, component::RigidBody& body) {
        auto* const bulletBody = _entities.getComponentSlow<BulletBody>(entityId);
        if (bulletBody == nullptr) {
            return;
        }

        _world.world.removeRigidBody(bulletBody->body);
        delete bulletBody->body;

        _entities.removeComponent<BulletBody>(entityId);
    }
} // namespace up

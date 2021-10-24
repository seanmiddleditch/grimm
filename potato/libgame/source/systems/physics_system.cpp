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
        class PhysicsSystem final : public System {
        public:
            explicit PhysicsSystem(Space& space);

            void update(float) override;

        private:
            btDefaultCollisionConfiguration _config;
            btCollisionDispatcher _dispatcher;
            btDbvtBroadphase _broadphase;
            btSequentialImpulseConstraintSolver _solver;
            btDiscreteDynamicsWorld _world;
            float _physicsTickRate = 1.f / 60.f;
        };
    } // namespace

    void registerPhysicsSystem(Space& space) { space.addSystem<PhysicsSystem>(); }

    PhysicsSystem::PhysicsSystem(Space& space)
        : System(space)
        , _dispatcher(&_config)
        , _world(&_dispatcher, &_broadphase, &_solver, &_config) { }

    void PhysicsSystem::update(float deltaTime) {
        // HACK
        //  initialize Bullet physics bodies
        //  TODO: efficiently query new components or teleports; apply initial rotation
        space().entities().select<component::Transform const, component::RigidBody>(
            [&](EntityId, component::Transform const& trans, component::RigidBody& body) {
                if (!body.body->isInWorld()) {
                    _world.addRigidBody(body.body.get());

                    btTransform& worldTrans = body.body->getWorldTransform();
                    worldTrans.setOrigin({trans.position.x, trans.position.y, trans.position.z});
                }
            });

        _world.stepSimulation(deltaTime, 12, _physicsTickRate);

        // Apply physics motion to transforms
        //  TODO: use btMotionState; apply rotation updates
        space().entities().select<component::Transform, component::RigidBody const>(
            [&](EntityId, component::Transform& trans, component::RigidBody const& body) {
                btTransform const& worldTrans = body.body->getWorldTransform();
                btVector3 const origin = worldTrans.getOrigin();
                trans.position = {origin.x(), origin.y(), origin.z()};

                if (trans.position.y <= 0.f) {
                    trans.position.y = 0.f;
                    body.body->setLinearVelocity({0, 0, 0});
                }
            });
    }
} // namespace up

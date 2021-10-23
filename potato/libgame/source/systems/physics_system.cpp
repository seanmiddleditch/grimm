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
            using System::System;

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
	
	void PhysicsSystem::start() {
	    space().world().createQuery(_bodiesQuery);
	}
	
	void PhysicsSystem::update(float deltaTime) {
	    _world.stepSimulation(deltaTime, 12, _physicsTickRate);
	
	    _bodiesQuery.select(space().world(), [&](EntityId, components::Transform& trans, components::Body& body) {
	        if (trans.position.y > 0) {
	            body.linearVelocity += glm::vec3(0.f, -10.f, 0.f) * deltaTime;
	            trans.position += body.linearVelocity * deltaTime;

                if (trans.position.y <= 0.f) {
                    trans.position.y = 0.f;
                    body.linearVelocity.y = 0.f;
                }
            }
        });
    }
} // namespace up

#include "player.h"
#include <QString>

Player::Player(glm::vec3 pos, const Terrain &terrain)
    : Entity(pos), m_velocity(0,0,0), m_acceleration(0,0,0),
      m_camera(pos + glm::vec3(0, 1.5f, 0)), mcr_terrain(terrain),
      mcr_camera(m_camera)
{}

Player::~Player()
{}

void Player::tick(float dT, InputBundle &input) {
    processInputs(input);
    computePhysics(dT, mcr_terrain, input);
}

void Player::processInputs(InputBundle &inputs) {
    // TODO: Update the Player's velocity and acceleration based on the
    // state of the inputs.

    this->m_acceleration = glm::vec3(0, 0, 0); // ensure we dont accidentally keep accelerating
    float tune_max_accel = 9.5; // this is acceleration of Usain Bolt

    // NOTE: it may help to check the note in the input bundle in the entity class to understand this line
    this->flightMode = inputs.flightMode; // make the player the opposite of what is set in the input bundle

    if (this->flightMode) { // if in flight mode

        // Update the players member variables. Go faster, turn off gravity
        this->gravity = 0;
        tune_max_accel *= 5; // this is acceleration of Usain Bolt

        if (inputs.wPressed) {
            this->m_acceleration += tune_max_accel * this->m_forward;
        }
        if (inputs.sPressed) {
            this->m_acceleration -= tune_max_accel * this->m_forward;
        }
        if (inputs.dPressed) {
            this->m_acceleration += tune_max_accel * this->m_right;
        }
        if (inputs.aPressed) {
            this->m_acceleration -= tune_max_accel * this->m_right;
        }
        if (inputs.ePressed) {
            this->m_acceleration += tune_max_accel * this->m_up;
        }
        if (inputs.qPressed) {
            this->m_acceleration -= tune_max_accel * this->m_up;
        }
        if (!inputs.wPressed && !inputs.sPressed && !inputs.dPressed && !inputs.aPressed && !inputs.ePressed && !inputs.qPressed) {
            this->m_velocity = glm::vec3(0, 0, 0); // can float and do nothing
            this->m_acceleration = glm::vec3(0, 0, 0); // can float and do nothing
        }

        // ensure we dont accelerate way too fast
        this->m_acceleration = glm::clamp(this->m_acceleration, -tune_max_accel, tune_max_accel);

    } else { // if not in flight mode

        // Update the players member variables. Ensure gravity is on
        float accel_scaler = 5; // to scale the ussain bolt and gravity values the same degree
        this->gravity = 9.81 * accel_scaler;
        tune_max_accel *= accel_scaler;

//        checkOnGround(inputs); // check if the player is on the ground

        if (inputs.wPressed) {
            this->m_acceleration += tune_max_accel * glm::vec3(this->m_forward.x, 0.f, this->m_forward.z); // zero out the y component of the acceleration
        }
        if (inputs.sPressed) {
            this->m_acceleration -= tune_max_accel * glm::vec3(this->m_forward.x, 0.f, this->m_forward.z); // zero out the y component of the acceleration
        }
        if (inputs.dPressed) {
            this->m_acceleration += tune_max_accel * glm::vec3(this->m_right.x, 0.f, this->m_right.z); // zero out the y component of the acceleration
        }
        if (inputs.aPressed) {
            this->m_acceleration -= tune_max_accel * glm::vec3(this->m_right.x, 0.f, this->m_right.z); // zero out the y component of the acceleration
        }

        if (inputs.spacePressed) {
//            std::cout << "On the ground? " << inputs.onGround << std::endl;
            // jump if on the ground
            if (inputs.onGround) {
//                std::cout << "I should be jumping" << std::endl;
                this->m_velocity.y = this->m_velocity.y + 100.f;
                inputs.onGround = false; // no longer on the ground (prevent flying)
            }
        }

        // include the gravity component
        this->m_acceleration -= glm::vec3(0, gravity, 0);

        // ensure we dont accelerate way too fast
        this->m_acceleration = glm::clamp(this->m_acceleration, -tune_max_accel, tune_max_accel);
    }

}

void Player::computePhysics(float dT, const Terrain &terrain, InputBundle &inputs) {
    // TODO: Update the Player's position based on its acceleration
    // and velocity, and also perform collision detection.
    // NOTE: This function is based off of kinematic equations

    float tune_max_speed = 15;

    if (this->flightMode) { // if in flight mode

        this->m_velocity *= 0.25f; // reduce velocity for friction and drag (slow down on release)
        this->m_velocity = this->m_velocity + this->m_acceleration * dT; // kinematics equation

        this->m_velocity = glm::clamp(this->m_velocity, -tune_max_speed, tune_max_speed); // ensure not too fast

        this->moveAlongVector(this->m_velocity * dT); // move along the position vector

    } else { // if not in flight mode

        tune_max_speed = 0.5 * tune_max_speed; // set the max speed for non-flight mode in the x and z directions
        this->m_velocity *= 0.25f; // reduce velocity for friction and drag (slow down on release)
        float terminal_speed = 10.f * tune_max_speed; // max speed for y; the terminal velocity of falling is normally 66m/s

        this->m_velocity = this->m_velocity + this->m_acceleration * dT; // kinematics equation for all dirs

        // Check if there is a collision
        glm::vec3 posRayDir = this->m_velocity * dT; // position vector
        checkCollision(posRayDir, terrain, inputs); // should edit the velocity to stop in the direction of collision

        // Ensure not going too fast
        glm::clamp(this->m_velocity.x, -tune_max_speed, tune_max_speed);
        glm::clamp(this->m_velocity.z, -tune_max_speed, tune_max_speed);
        glm::clamp(this->m_velocity.y, -terminal_speed, terminal_speed);

        this->moveAlongVector(this->m_velocity * dT); // move along the position vector
    }
}

bool Player::gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit) {
    float maxLen = glm::length(rayDirection); // Farthest we search
    glm::ivec3 currCell = glm::ivec3(glm::floor(rayOrigin));
    rayDirection = glm::normalize(rayDirection); // Now all t values represent world dist.

    float curr_t = 0.f;
    while(curr_t < maxLen) {
        float min_t = glm::sqrt(3.f);
        float interfaceAxis = -1; // Track axis for which t is smallest
        for(int i = 0; i < 3; ++i) { // Iterate over the three axes to fing the closest point of intersection with the block grid
            if(rayDirection[i] != 0) { // Is ray parallel to axis i?
                float offset = glm::max(0.f, glm::sign(rayDirection[i])); // See slide 5
                // If the player is *exactly* on an interface then
                // they'll never move if they're looking in a negative direction
                if(currCell[i] == rayOrigin[i] && offset == 0.f) {
                    offset = -1.f;
                }
                int nextIntercept = currCell[i] + offset;
                float axis_t = (nextIntercept - rayOrigin[i]) / rayDirection[i];
                axis_t = glm::min(axis_t, maxLen); // Clamp to max len to avoid super out of bounds errors
                if(axis_t < min_t) { // determinging the cell we are intersecting from the point on the axis of intersection
                    min_t = axis_t; // this is now the closes acis intersection
                    interfaceAxis = i; // track axis for which t is smallest
                }
            }
        }
        if(interfaceAxis == -1) {
            throw std::out_of_range("interfaceAxis was -1 after the for loop in gridMarch!");
        }
        curr_t += min_t; // min_t is declared in slide 7 algorithm
        rayOrigin += rayDirection * min_t;
        glm::ivec3 offset = glm::ivec3(0,0,0);
        // Sets it to 0 if sign is +, -1 if sign is -
        offset[interfaceAxis] = glm::min(0.f, glm::sign(rayDirection[interfaceAxis]));
        currCell = glm::ivec3(glm::floor(rayOrigin)) + offset;
        // If currCell contains something other than EMPTY, return
        // curr_t
        BlockType cellType = terrain.getBlockAt(currCell.x, currCell.y, currCell.z);
        if(cellType != EMPTY) {
            *out_blockHit = currCell;
            *out_dist = glm::min(maxLen, curr_t);
            return true;
        }
    }
    *out_dist = glm::min(maxLen, curr_t);
    return false;
}

void Player::checkCollision(glm::vec3 &rayDirection, const Terrain &terrain, InputBundle &inputs)
{
    // NOTE: the ray direction is the position based on the velocity because in gridmarch we check as far as this vector!
    glm::vec3 playerVertOrigin = glm::vec3(this->m_position); // effetively the bottom left vertex of the 2 blocks stack (player)
    float out_dist = 0; // delcare input to grid march (how far away the collision is to the block, if at all)
    glm::ivec3 out_blockHit = glm::ivec3(); // declare the input to grid march (cell that we are colliding with, if any)
    // make rays for every corner of the 2 block stack and check for collisions
    for (float x = -0.5; x <= 0.5; x++) { // iterate over the 2 possible x coords
        for (int y = 0; y < 3; y++) { // iterate over the 2 possible y coords (recall the 2 block stack! Hence 3)
            for (float z = -0.5; z <= 0.5; z++) { // iterate over the possible z coords
                glm::vec3 castedRayOrigin = glm::vec3(playerVertOrigin.x + x,
                                                      playerVertOrigin.y + y,
                                                      playerVertOrigin.z + z); // z is negative because the forward dir is -z

                // positive direction collisions
                glm::vec3 rayX = rayDirection * glm::vec3(1, 0, 0); // get only the x component
                glm::vec3 rayY = rayDirection * glm::vec3(0, 1, 0); // get only the y component
                glm::vec3 rayZ = rayDirection * glm::vec3(0, 0, 1); // get only the z component
                if (gridMarch(castedRayOrigin, rayX, terrain, &out_dist, &out_blockHit)) { // if there is a collision in x
                    this->m_velocity.x = 0; // ensure you cant move in this dir
                }
                if (gridMarch(castedRayOrigin, rayY, terrain, &out_dist, &out_blockHit)) { // if there is a collision in y
                    this->m_velocity.y = 0; // ensure you cant move in this dir
                    if (y == 0) { // set the ground bool to true
                        inputs.onGround = true;
//                        this->m_position.y = glm::floor(this->m_position).y;
                    }
                }
                if (gridMarch(castedRayOrigin, rayZ, terrain, &out_dist, &out_blockHit)) { // if there is a collision in z
                    this->m_velocity.z = 0; // ensure you cant move in this dir
                }
            }
        }
    }
}

//bool Player::checkOnGround(InputBundle &inputs)
//{
//    // Get the block type below the player (just barely below the foot of the player) to see if on the ground
//    BlockType blockBelow = this->mcr_terrain.getBlockAt(glm::floor(this->m_position.x), this->m_position.y - 0.05, glm::floor(this->m_position.z));

//    if (blockBelow != EMPTY) {
//        inputs.onGround = true;
//    } else {
//        inputs.onGround = false;
//    }

//    return inputs.onGround;
//}

void Player::setCameraWidthHeight(unsigned int w, unsigned int h) {
    m_camera.setWidthHeight(w, h);
}

void Player::moveAlongVector(glm::vec3 dir) {
    Entity::moveAlongVector(dir);
    m_camera.moveAlongVector(dir);
}
void Player::moveForwardLocal(float amount) {
    Entity::moveForwardLocal(amount);
    m_camera.moveForwardLocal(amount);
}
void Player::moveRightLocal(float amount) {
    Entity::moveRightLocal(amount);
    m_camera.moveRightLocal(amount);
}
void Player::moveUpLocal(float amount) {
    Entity::moveUpLocal(amount);
    m_camera.moveUpLocal(amount);
}
void Player::moveForwardGlobal(float amount) {
    Entity::moveForwardGlobal(amount);
    m_camera.moveForwardGlobal(amount);
}
void Player::moveRightGlobal(float amount) {
    Entity::moveRightGlobal(amount);
    m_camera.moveRightGlobal(amount);
}
void Player::moveUpGlobal(float amount) {
    Entity::moveUpGlobal(amount);
    m_camera.moveUpGlobal(amount);
}
void Player::rotateOnForwardLocal(float degrees) {
    Entity::rotateOnForwardLocal(degrees);
    m_camera.rotateOnForwardLocal(degrees);
}
void Player::rotateOnRightLocal(float degrees) {
    Entity::rotateOnRightLocal(degrees);
    m_camera.rotateOnRightLocal(degrees);
}
void Player::rotateOnUpLocal(float degrees) {
    Entity::rotateOnUpLocal(degrees);
    m_camera.rotateOnUpLocal(degrees);
}
void Player::rotateOnForwardGlobal(float degrees) {
    Entity::rotateOnForwardGlobal(degrees);
    m_camera.rotateOnForwardGlobal(degrees);
}
void Player::rotateOnRightGlobal(float degrees) {
    Entity::rotateOnRightGlobal(degrees);
    m_camera.rotateOnRightGlobal(degrees);
}
void Player::rotateOnUpGlobal(float degrees) {
    Entity::rotateOnUpGlobal(degrees);
    m_camera.rotateOnUpGlobal(degrees);
}

QString Player::posAsQString() const {
    std::string str("( " + std::to_string(m_position.x) + ", " + std::to_string(m_position.y) + ", " + std::to_string(m_position.z) + ")");
    return QString::fromStdString(str);
}
QString Player::velAsQString() const {
    std::string str("( " + std::to_string(m_velocity.x) + ", " + std::to_string(m_velocity.y) + ", " + std::to_string(m_velocity.z) + ")");
    return QString::fromStdString(str);
}
QString Player::accAsQString() const {
    std::string str("( " + std::to_string(m_acceleration.x) + ", " + std::to_string(m_acceleration.y) + ", " + std::to_string(m_acceleration.z) + ")");
    return QString::fromStdString(str);
}
QString Player::lookAsQString() const {
    std::string str("( " + std::to_string(m_forward.x) + ", " + std::to_string(m_forward.y) + ", " + std::to_string(m_forward.z) + ")");
    return QString::fromStdString(str);
}



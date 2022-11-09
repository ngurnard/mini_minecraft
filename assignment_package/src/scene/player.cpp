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
        this->accel_max = 1.5 * tune_max_accel;

        if (inputs.wPressed) {
            this->m_acceleration += this->m_forward;
        }
        if (inputs.sPressed) {
            this->m_acceleration -= this->m_forward;
        }
        if (inputs.dPressed) {
            this->m_acceleration += this->m_right;
        }
        if (inputs.aPressed) {
            this->m_acceleration -= this->m_right;
        }
        if (inputs.ePressed) {
            this->m_acceleration += this->m_up;
        }
        if (inputs.qPressed) {
            this->m_acceleration -= this->m_up;
        }
        if (!inputs.wPressed && !inputs.sPressed && !inputs.dPressed && !inputs.aPressed && !inputs.ePressed && !inputs.qPressed) {
            this->m_velocity = glm::vec3(0, 0, 0); // can float and do nothing
            this->m_acceleration = glm::vec3(0, 0, 0); // can float and do nothing
        }

    } else { // if not in flight mode

        // Update the players member variables. Ensure gravity is on
        this->gravity = 9.81;
        this->accel_max = tune_max_accel; // acceleration of Usain Bolt

        checkOnGround(inputs); // check if the player is on the ground

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
            // jump if on the ground
            if (inputs.onGround) {
                this->m_velocity.y = this->m_velocity.y + 10.f;
            }
        }
        if (inputs.onGround) {
            this->m_velocity.y = 0; // the player cant fall through the ground
        }

    }

    // if the normalize function doesn't return nans (typically for zero vector or huge vector)
    bool check_nans = glm::any(glm::isnan(glm::normalize(m_acceleration))); // true for nans
    if (!check_nans && flightMode) { // if in flight mode
        this->m_acceleration = this->accel_max * glm::normalize(m_acceleration); // max out the acceleration to the norm max
    } else if (!flightMode) { // if not in flight mode
        if (!check_nans) {
            this->m_acceleration = this->accel_max * glm::normalize(m_acceleration); // max out the acceleration to the norm max
            // if on the ground, kepp, else make this smaller!
            // include a constant downward acceleration due to gravity
            this->m_acceleration -= glm::vec3(0, gravity, 0);
        } else {
            this->m_acceleration -= glm::vec3(0, gravity, 0); // just make it fall straight down
        }
    } else {
        this->m_acceleration = glm::vec3(0, 0, 0); // fail safe
    }

}

void Player::computePhysics(float dT, const Terrain &terrain, InputBundle &inputs) {
    // TODO: Update the Player's position based on its acceleration
    // and velocity, and also perform collision detection.
    // NOTE: This function is based off of kinematic equations

    float tune_max_speed = 0.5;

//    std::cout << "Acceleration in compute physics: {" << m_acceleration.x << ", " << m_acceleration.y << ", " << m_acceleration.z << "}" << std::endl;
    if (this->flightMode) { // if in flight mode
        this->speed_max = 2.f * tune_max_speed; // set the max speed for flight mode

        this->m_velocity = this->m_velocity + this->m_acceleration * dT; // kinematics equation

        // if the normalize function doesn't return nans (typically for zero vector or huge vector) and if not going too fast
        bool check_nans = glm::any(glm::isnan(glm::normalize(m_velocity)));
        if (!check_nans && glm::length(m_velocity) < speed_max) {
            this->m_velocity = this->speed_max * this->m_velocity; // allow it to build up speed
        } else if (check_nans) {
            this->m_velocity = glm::vec3(0, 0, 0); // fail safe
        } else {
            this->m_velocity = this->speed_max * glm::normalize(this->m_velocity); // cap at max speed
        }

        this->moveAlongVector(this->m_velocity);

    } else { // if not in flight mode

        this->speed_max = 1.f * tune_max_speed; // set the max speed for non-flight mode in the x and z directions
        this->m_velocity *= 0.65f; // reduce velocity for friction and drag (slow down on release)

        // check if on the ground, if yes, stop falling
        if (inputs.onGround) {
            this->m_velocity.x = this->m_velocity.x + this->m_acceleration.x * dT; // kinematics equation for x
            this->m_velocity.z = this->m_velocity.z + this->m_acceleration.z * dT; // kinematics equation for z
            this->m_velocity.y = 0; // no y velocity

            // Check if there is a collision


            // if the normalize function doesn't return nans (typically for zero vector or huge vector) and if not going too fast
            bool check_nans = glm::any(glm::isnan(glm::normalize(m_velocity)));
            if (!check_nans && glm::length(m_velocity) < speed_max) {
                // do nothing
            } else if (check_nans) {
                this->m_velocity = glm::vec3(0, 0, 0); // fail safe
            } else {
                this->m_velocity = this->speed_max * glm::normalize(this->m_velocity); // cap at max speed
            }

        } else {

            float terminal_speed = 2.f * tune_max_speed; // max speed for y; the terminal velocity of falling is normally 66m/s

            this->m_velocity = this->m_velocity + this->m_acceleration * dT; // kinematics equation for all dirs

            // Check if there is a collision
            glm::vec3 rayDir = this->m_velocity;
            checkCollision(&rayDir, terrain);

            // if the normalize function doesn't return nans (typically for zero vector or huge vector) and if not going too fast
            bool check_nans = glm::any(glm::isnan(glm::normalize(m_velocity)));
            if (!check_nans && glm::length(m_velocity) < speed_max) {
                //do nothing
            } else if (check_nans) {
                this->m_velocity = glm::vec3(0, 0, 0); // fail safe
            } else {
                glm::vec3 temp_vel = glm::normalize(this->m_velocity);
                this->m_velocity.x = this->speed_max * temp_vel.x; // cap at max speed in x
                this->m_velocity.z = this->speed_max * temp_vel.z; // cap at max speed in z
                this->m_velocity.y = terminal_speed * temp_vel.y; // cap at max speed in y
            }

        }
        this->moveAlongVector(this->m_velocity);

    }

}

bool gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit) {
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
                if(axis_t < min_t) {
                    min_t = axis_t;
                    interfaceAxis = i;
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

void Player::checkCollision(glm::vec3 *rayDirection, const Terrain &terrain)
{
    // idk yet
}

bool Player::checkOnGround(InputBundle &inputs)
{
    // Get the block type below the player (just barely below the foot of the player) to see if on the ground
    BlockType blockBelow = this->mcr_terrain.getBlockAt(glm::floor(this->m_position.x), this->m_position.y - 0.05, glm::floor(this->m_position.z));

    if (blockBelow != EMPTY) {
        inputs.onGround = true;
    } else {
        inputs.onGround = false;
    }

    return inputs.onGround;
}

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



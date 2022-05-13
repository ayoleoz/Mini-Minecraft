#include "player.h"
#include <QString>
#include <iostream>

Player::Player(glm::vec3 pos, const Terrain &terrain)
    : Entity(pos), m_prevPos(pos), m_velocity(0,0,0), m_acceleration(0,0,0),
      m_camera(pos + glm::vec3(0, 1.5f, 0)), mcr_terrain(terrain),
      faceAxis(-1), mcr_prevPos(m_prevPos), mcr_camera(m_camera), isInWater(false), isInLava(false)
{}

Player::~Player()
{}

void Player::tick(float dT, InputBundle &input) {
    m_prevPos = m_position;
    processInputs(input);
    computePhysics(dT, mcr_terrain);

    // check whether the player is in water or lava
    // and set isInWater or isInLava to true or false
    checkInWater();
    checkInLava();
}

void Player::processInputs(InputBundle &inputs) {
    // TODO: Update the Player's velocity and acceleration based on the
    // state of the inputs.
    m_acceleration = glm::vec3(0.f,0.f,0.f);

    // check flightMode
    if (inputs.flightMode) {
        this->flightMode = true;

        if (inputs.wPressed) {
            this->m_acceleration = this->acc * glm::normalize(this->m_forward);
        } else if (inputs.sPressed) {
            this->m_acceleration = this->acc * -1.f * glm::normalize(this->m_forward);
        } else if (inputs.aPressed) {
            this->m_acceleration = this->acc * -1.f * glm::normalize(this->m_right);
        } else if (inputs.dPressed) {
            this->m_acceleration = this->acc * glm::normalize(this->m_right);
        } else if (inputs.qPressed) {
            this->m_acceleration = this->acc * glm::normalize(this->m_up);
        } else if (inputs.ePressed) {
            this->m_acceleration = this->acc * -1.f * glm::normalize(this->m_up);
        } else {
            this->m_velocity = glm::vec3(0, 0, 0);
            this->m_acceleration = glm::vec3(0, 0, 0);
        }
    } else {
        this->flightMode = false;

        if (inputs.wPressed) {
            this->m_acceleration = this->acc * glm::normalize(glm::vec3(this->m_forward.x, 0.f, this->m_forward.z));
        } else if (inputs.sPressed) {
            this->m_acceleration = this->acc * -1.f * glm::normalize(glm::vec3(this->m_forward.x, 0.f, this->m_forward.z));
        } else if (inputs.aPressed) {
            this->m_acceleration = this->acc * -1.f * glm::normalize(glm::vec3(this->m_right.x, 0.f, this->m_right.z));
        } else if (inputs.dPressed) {
            this->m_acceleration = this->acc * glm::normalize(glm::vec3(this->m_right.x, 0.f, this->m_right.z));
        }
        if (inputs.spacePressed)
        {
            if (!(isInWater || isInLava))
            {
                // only jump if there is something below
                BlockType belowPlayer = this->mcr_terrain.getBlockAt(this->m_position.x - 0.5, this->m_position.y - 0.1, this->m_position.z - 0.5);
                if (belowPlayer != EMPTY && belowPlayer != WATER && belowPlayer != LAVA) {
                    this->m_velocity.y = 15.f;
                }
                belowPlayer = this->mcr_terrain.getBlockAt(this->m_position.x + 0.5, this->m_position.y - 0.1, this->m_position.z - 0.5);
                if (belowPlayer != EMPTY && belowPlayer != WATER && belowPlayer != LAVA) {
                    this->m_velocity.y = 15.f;
                }
                belowPlayer = this->mcr_terrain.getBlockAt(this->m_position.x + 0.5, this->m_position.y - 0.1, this->m_position.z + 0.5);
                if (belowPlayer != EMPTY && belowPlayer != WATER && belowPlayer != LAVA) {
                    this->m_velocity.y = 15.f;
                }
                belowPlayer = this->mcr_terrain.getBlockAt(this->m_position.x - 0.5, this->m_position.y - 0.1, this->m_position.z + 0.5);
                if (belowPlayer != EMPTY && belowPlayer != WATER && belowPlayer != LAVA) {
                    this->m_velocity.y = 15.f;
                }
            }
            // When the player is in water or lava,
            // the player should move at 2/3 its normal speed (both in terms of gravity and lateral movement)
            // and should be able to swim upwards at a constant rate by holding Spacebar
            else
            {
                this->m_acceleration = this->acc * glm::normalize(this->m_up);
                this->m_velocity.y = 3.f;
            }
        }
    }
}

void Player::computePhysics(float dT, const Terrain &terrain) {
    // TODO: Update the Player's position based on its acceleration
    // and velocity, and also perform collision detection.
    if (this->flightMode) {
        this->m_velocity *= 0.95;
        this->m_velocity += this->m_acceleration * dT;

        glm::vec3 rayDirection = this->m_velocity * dT;
        //detectCollision(&rayDirection, terrain);

        if (isInWater || isInLava)
        {
            rayDirection *= 2.f / 3.f;
        }

        this->moveAlongVector(rayDirection);
    } else {
        BlockType belowPlayer = this->mcr_terrain.getBlockAt(this->m_position.x, this->m_position.y - 0.1, this->m_position.z);
        if (dT == 0 && (belowPlayer == EMPTY || belowPlayer == WATER || belowPlayer == LAVA)) {
            dT = 1.f;
        }
        if (belowPlayer == EMPTY)
        {
            this->m_acceleration += glm::vec3(0, -3 * this->g, 0);
        }
        if (belowPlayer == WATER || belowPlayer == LAVA)
        {
            this->m_acceleration += glm::vec3(0, -2 * this->g, 0);
        }
        this->m_velocity *= 0.95;
        this->m_velocity += this->m_acceleration * dT;

        glm::vec3 rayDirection = this->m_velocity * dT;

        if (isInWater || isInLava)
        {
            rayDirection *= 2.f / 3.f;
        }

        detectCollision(&rayDirection, terrain);
        this->moveAlongVector(rayDirection);
    }
}

// from slides
bool Player::gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit) {
    float maxLen = glm::length(rayDirection); // Farthest we search
        glm::ivec3 currCell = glm::ivec3(glm::floor(rayOrigin));
        rayDirection = glm::normalize(rayDirection); // Now all t values represent world dist.

        float curr_t = 0.f;
        while(curr_t < maxLen) {
            float min_t = glm::sqrt(3.f);
            float interfaceAxis = -1; // Track axis for which t is smallest
            for(int i = 0; i < 3; ++i) { // Iterate over the three axes
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
            faceAxis = interfaceAxis;
            curr_t += min_t; // min_t is declared in slide 7 algorithm
            rayOrigin += rayDirection * min_t;
            glm::ivec3 offset = glm::ivec3(0,0,0);
            // Sets it to 0 if sign is +, -1 if sign is -
            offset[interfaceAxis] = glm::min(0.f, glm::sign(rayDirection[interfaceAxis]));
            currCell = glm::ivec3(glm::floor(rayOrigin)) + offset;
            // If currCell contains something other than EMPTY, return
            // curr_t
            BlockType cellType = terrain.getBlockAt(currCell.x, currCell.y, currCell.z);
            if(cellType != EMPTY && cellType != WATER && cellType != LAVA) {
                *out_blockHit = currCell;
                *out_dist = glm::min(maxLen, curr_t);
                return true;
            }
        }
        *out_dist = glm::min(maxLen, curr_t);
        return false;
}

void Player::detectCollision(glm::vec3 *rayDirection, const Terrain &terrain) {
    glm::vec3 origin = m_position - glm::vec3(0.5f, 0.f, 0.5f);
    glm::ivec3 outBlockHit = glm::ivec3();
    float outDist = 0.f;
    float min_x = rayDirection->x;
    float min_y = rayDirection->y;
    float min_z = rayDirection->z;
    for (int x = 0; x <= 1; x++) {
        for (int z = 0; z <= 1; z++) {
            for (int y = 0; y <= 2; y++) {
                glm::vec3 rayOrigin = origin + glm::vec3(x, y, z);
                if (gridMarch(rayOrigin, glm::vec3(rayDirection->x,0,0), terrain, &outDist, &outBlockHit)) {
                    if(outDist > 0.005){
                        if(glm::abs(min_x) > outDist - 0.005)
                        {
                            min_x = (outDist - 0.005) * glm::sign(min_x);
                        }
                    }
                    else{
                        min_x = 0;
                    }
                }
                if (gridMarch(rayOrigin, glm::vec3(0,rayDirection->y,0), terrain, &outDist, &outBlockHit)) {
                    if(outDist > 0.005){
                        if(glm::abs(min_y) > outDist - 0.005)
                        {
                            min_y = (outDist - 0.005) * glm::sign(min_y);
                        }
                    }
                    else{
                        min_y = 0;
                    }
                }
                if (gridMarch(rayOrigin, glm::vec3(0,0,rayDirection->z), terrain, &outDist, &outBlockHit)) {
                    if(outDist > 0.005){
                        if(glm::abs(min_z) > outDist - 0.005)
                        {
                            min_z = (outDist - 0.005) * glm::sign(min_z);
                        }
                    }
                    else{
                        min_z = 0;
                    }
                }
            }
        }
    }
    rayDirection->x = min_x;
    rayDirection->y = min_y;
    rayDirection->z = min_z;
}

BlockType Player::placeBlock(Terrain *t, BlockType currBlockType) {
    glm::vec3 rayOrigin = this->m_camera.mcr_position;
    glm::vec3 rayDirection = 3.f * glm::normalize(this->m_forward);
    glm::ivec3 out_blockHit = glm::ivec3();
    float out_dist = 0.f;

    if (!gridMarch(rayOrigin, rayDirection, this->mcr_terrain, &out_dist, &out_blockHit)) {
        out_blockHit = this->m_camera.mcr_position + rayDirection;
        Chunk* c = t->getChunkAt(out_blockHit.x, out_blockHit.z).get();
        glm::vec2 chunkOrigin = glm::vec2(floor(out_blockHit.x / 16.f) * 16, floor(out_blockHit.z / 16.f) * 16);

        c->setBlockAt(static_cast<unsigned int>(out_blockHit.x - chunkOrigin.x),
                             static_cast<unsigned int>(out_blockHit.y),
                             static_cast<unsigned int>(out_blockHit.z - chunkOrigin.y), currBlockType);
//         TODO:
        t->getChunkAt(out_blockHit.x, out_blockHit.z).get()->destroy();
        t->spawnVBOWorker(t->getChunkAt(out_blockHit.x, out_blockHit.z).get());
        return currBlockType;
    }
    return EMPTY;
}

BlockType Player::removeBlock(Terrain *t) {
    glm::vec3 rayOrigin = this->m_camera.mcr_position;
    glm::vec3 rayDirection = 3.f * glm::normalize(this->m_forward);
    glm::ivec3 out_blockHit = glm::ivec3();
    float out_dist = 0.f;

    if (gridMarch(rayOrigin, rayDirection, this->mcr_terrain, &out_dist, &out_blockHit)) {
        BlockType blockType = t->getBlockAt(out_blockHit.x, out_blockHit.y, out_blockHit.z);
        t->setBlockAt(out_blockHit.x, out_blockHit.y, out_blockHit.z, EMPTY);
        t->getChunkAt(out_blockHit.x, out_blockHit.z).get()->destroy();
        t->spawnVBOWorker(t->getChunkAt(out_blockHit.x, out_blockHit.z).get());
        return blockType;
    }
    return EMPTY;
}


void Player::checkInWater()
{
    BlockType currBlockType = this->mcr_terrain.getBlockAt(this->m_position.x, this->m_position.y + 1, this->m_position.z);
    if (currBlockType == WATER)
    {
        this->isInWater = true;
    }
    else
    {
        this->isInWater = false;
    }
}

bool Player::checkInWaterLow()
{
    BlockType currBlockType = this->mcr_terrain.getBlockAt(this->m_position.x, this->m_position.y + 1.5, this->m_position.z);
    if (currBlockType == WATER)
    {
        return true;
    }
    return false;
}

void Player::checkInLava()
{
    BlockType currBlockType = this->mcr_terrain.getBlockAt(this->m_position.x, this->m_position.y + 1, this->m_position.z);
    if (currBlockType == LAVA)
    {
        this->isInLava = true;
    }
    else
    {
        this->isInLava = false;
    }
}

bool Player::checkInLavaLow()
{
    BlockType currBlockType = this->mcr_terrain.getBlockAt(this->m_position.x, this->m_position.y + 1.5, this->m_position.z);
    if (currBlockType == LAVA)
    {
        return true;
    }
    return false;
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

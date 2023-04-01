/*
 * Created by Brett on 30/03/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */

#ifndef ASSIGN3_PARTICLE_SYSTEM_H
#define ASSIGN3_PARTICLE_SYSTEM_H

#include <texture.h>
#include <unordered_map>
#include <blt/std/random.h>
#include <vector>
#include <queue>
#include <algorithm>
#include <util.h>
#include <sstream>
#include <config.h>

struct particle_t {
    vec pos, dir;
    float speed;
    float age;
};

class particle_system {
    private:
        std::unordered_map<unsigned int, std::vector<particle_t*>> particles;
        blt::vec3 position;
        blt::vec3 direction;
        float spread;
        int pps;
        // current number of particles spawned in the last second
        int current_pps = 0;
        // delta to keep track of pps relative to the framerate
        double delta = 0;
        enum status_t {
            CONTINUOUS = 0, MANUAL = 1, SINGLE = 2
        };
        enum spray_status_t {
            DEFAULT = 0, LOW = 1, HIGH = 2
        };
        
        status_t status = CONTINUOUS;
        spray_status_t sprayMode = DEFAULT;
        const float sprays[3] = {1, 5, 10};
        
        bool applyFriction = true;
        bool randomizeTexture = false;
        
        blt::random<float> DIRECTION_RANDOMIZER{-1, 1};
        blt::random<float> TEXTURE_RANDOMIZER{0, 9};
        
        void spawnParticle() {
            auto* p = new particle_t;
            
            constexpr float SPEED_FACTOR = 25;
            
            unsigned int textureID = 0;
            
            if (randomizeTexture)
                textureID = (int) round(TEXTURE_RANDOMIZER.get());
            
            p->pos = conv(position);
            auto offset = blt::vec3{DIRECTION_RANDOMIZER.get(), 0, DIRECTION_RANDOMIZER.get()};
            p->dir = conv(direction * SPEED_FACTOR + offset.normalize() * spread * sprays[sprayMode]);
            p->speed = 1.0;
            p->age = 0;
            
            particles[textureID].push_back(p);
        }
        
        inline static double distance(const particle_t* p, const blt::vec3& pos){
            const auto ppos = p->pos;
            auto dx = ppos.x - pos.x();
            auto dy = ppos.y - pos.y();
            auto dz = ppos.z - pos.z();
            return dx * dx + dy * dy + dz * dz;
        }
        
        // vbo stuff
        // if I had access to GL3.3+ I could instance the particles which would be much faster to render
        // it wouldn't be hard as BLT has most of the math functions required and I could steal the
        // shader loader / VAO object implementation from my final project but that's a TODO:
        // the hard part is glut's apparent lack of gl3+ support
        // -- apparently needed glad for this, maybe it is possible without switching to glfw
        unsigned int quad = 0;
        
        const float s = 0.5;
        const float PARTICLE_LIFETIME = 25;
    public:
        particle_system(
                const blt::vec3& position, const blt::vec3& direction, float spread, int pps
        ): position(position), direction(direction), spread(spread), pps(pps) {
#ifndef EXTRAS
            quad = glGenLists(1);
            glNewList(quad, GL_COMPILE);
            glBegin(GL_QUADS);
            glTexCoord2f(1, 0);
            glVertex3f(-s, s, 0);
            glTexCoord2f(1, 1);
            glVertex3f(-s, -s, 0);
            glTexCoord2f(0, 1);
            glVertex3f(s, -s, 0);
            glTexCoord2f(0, 0);
            glVertex3f(s, s, 0);
            glEnd();
            glEndList();
#endif
        }
        
        void update(camera& cam, float bnx, float bnz, float bpx, float bpz) {
            // used to keep updates correct without being dependent on FPS
            auto frameDeltaSeconds = ((double) getDelta() / 1000000000.0);
            const auto gravity_vec = vec{0, -GRAVITY, 0};
            
            for (auto& pair : particles) {
                std::queue<particle_t*> deleteList;
                for (auto& particle : pair.second) {
                    auto ppos = particle->pos;
                    particle->pos = ppos + particle->dir * particle->speed * frameDeltaSeconds;
                    particle->age += (float) frameDeltaSeconds;
                    
                    float BOUNCE_FACTOR = 0.75;
                    
                    // bounce particle
                    if (particle->pos.y < 0 && particle->pos.x > bnx && particle->pos.x < bpx &&
                        particle->pos.z > bnz && particle->pos.z < bpz) {
                        // NOTE: I am not deleting stationary particles because I think they look cool sitting on the floor.
                        if (!applyFriction)
                            BOUNCE_FACTOR = 1.0;
                        particle->dir.y = -particle->dir.y * BOUNCE_FACTOR;
                        particle->pos = ppos;
                    }
                    
                    // remove particles outside the bounds of the world
                    if (particle->pos.y < -50 || particle->age > PARTICLE_LIFETIME) {
                        deleteList.push(particle);
                        continue;
                    }
                    
                    particle->dir = particle->dir + gravity_vec * frameDeltaSeconds;
                }
                while (!deleteList.empty()) {
                    auto*& particle = deleteList.front();
                    
                    auto itr = std::find(pair.second.begin(), pair.second.end(), particle);
                    if (itr != pair.second.end())
                        pair.second.erase(itr);
                    
                    delete (particle);
                    
                    deleteList.pop();
                }
            }
            int particlesToSpawn = (int) (round(pps * frameDeltaSeconds));
            
            if (current_pps + particlesToSpawn > pps)
                particlesToSpawn = pps - current_pps;
            
            if (status == CONTINUOUS) {
                for (int i = 0; i < particlesToSpawn; i++)
                    spawnParticle();
            } else if (status == MANUAL) {
                if (cam.isKeyPressed('f'))
                    for (int i = 0; i < particlesToSpawn; i++)
                        spawnParticle();
            }
            
            delta += frameDeltaSeconds;
            if (delta > 1.0) {
                current_pps = 0;
                delta = 0;
            }
        }
        
        inline static void applyBillboard() {
#ifndef EXTRAS
            GLfloat m[16];
            glMatrixMode(GL_MODELVIEW);
            glGetFloatv(GL_MODELVIEW_MATRIX, m);
            // undo all rotations (and scaling!) by setting rotation part to the identity matrix
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (i == j)
                        m[i * 4 + j] = 1.0;
                    else
                        m[i * 4 + j] = 0.0;
                }
            }
            
            glLoadMatrixf(m);
#endif
        }
        
        void render(camera& cam, texture** textures) {
#ifndef EXTRAS
            glMatrixMode(GL_MODELVIEW);
            // by batching particles by texture we save a little driver overhead
            for (auto& pair : particles) {
                //glBindTexture(GL_TEXTURE_2D, );
                textures[pair.first]->bind();
                auto& particles_vec = pair.second;
                
                // since we have enabled transparency, we need to sort the particles based on the distance to the camera.
                // TODO: this does not account for bill-boarding / rotation!
                // TODO: std::sort is likely a quick-sort derivative, insertion sort would be better as particles are always* mostly* sorted!
                std::sort(particles_vec.begin(), particles_vec.end(), [&](const particle_t* p1, const particle_t* p2) {
                    auto d1 = distance(p1, cam.getPosition());
                    auto d2 = distance(p2, cam.getPosition());
                    return d1 > d2;
                });
                
                for (auto& p : particles_vec) {
                    glPushMatrix();
                    glTranslatef(p->pos.x, p->pos.y, p->pos.z);
                    
                    // due to the inefficiency of getting the modelview and updating it to billboard,
                    // this will cause a considerable slow down.
                    applyBillboard();
                    
                    glCallList(quad);
                    
                    glPopMatrix();
                }
            }
            
            auto err = glGetError();
            if (err != 0)
                std::cout << "GL Error: " << err << "\n";
            {
                std::stringstream str;
                str << WINDOW_TITLE;
                str << " | Particles: ";
                size_t count = 0;
                for (const auto& pair : particles)
                    count += pair.second.size();
                str << count;
                str << " FPS: ";
                str << 1000000000.0 / (double)getDelta();
                str << " | Status: ";
                str << (status == CONTINUOUS ? "Continuous" : status == MANUAL ? "Manual" : "Single");
                str << " | Friction: ";
                str << (applyFriction ? "True" : "False");
                str << " | Spray: ";
                str << (sprayMode == DEFAULT ? "Default" : sprayMode == LOW ? "Low" : "High");
                str << " | Randomize: ";
                str << (randomizeTexture ? "True" : "False");
                glutSetWindowTitle(str.str().c_str());
            }
#endif
        }
        
        void randomizeSpeed(float n) {
            blt::random<float> SPEED_RANDOMIZER{1, n};
            
            for (auto& pair : particles)
                for (auto& p : pair.second)
                    p->speed = SPEED_RANDOMIZER.get();
        }
        
        void setFriction(bool f){
            applyFriction = f;
        }
        
        [[nodiscard]] bool getFriction() const {
            return applyFriction;
        }
        
        void toggleSpray(){
            int currentSpray = sprayMode;
            // increment and bound the spray pattern
            currentSpray++;
            if (currentSpray > 2)
                currentSpray = 0;
            sprayMode = static_cast<spray_status_t>(currentSpray);
        }
        
        void toggleTexRandomizer() {
            randomizeTexture = !randomizeTexture;
        }
        
        void changeFiringMode(int mode){
            // wrap
            if (mode < 0)
                mode = SINGLE;
            if (mode > 2)
                mode = CONTINUOUS;
            // convert
            status = static_cast<status_t>(mode);
        }
        
        int getFiringMode(){
            return status;
        }
        
        void singleFire(){
            BLT_TRACE("Hello!");
            if (status == SINGLE)
                spawnParticle();
        }
        
        void reset(){
            particles.clear();
        }
        
        ~particle_system() {
            glDeleteLists(quad, 1);
            for (const auto& pair : particles) {
                for (auto& particle : pair.second)
                    delete (particle);
            }
        }
};

#endif //ASSIGN3_PARTICLE_SYSTEM_H

#include "ParticleEmitter.h"
#include <cstdlib>
#include <iostream>

ParticleEmitter::ParticleEmitter(ShaderProgram *shaderProgram,
                                 Texture *texture,
                                 GLuint amount)
    : shaderProgram_(shaderProgram), texture_(texture), amount_(amount), lastUsedParticle_(0) {
    std::cout << "ParticleEmitter constructor" << std::endl;

    GLfloat quad[] = {
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };

    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO_);
    glBindVertexArray(VAO_);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid *) 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    for (GLuint i = 0; i < amount_; i++) {
        particles_.push_back(Particle());
    }
}

ParticleEmitter::~ParticleEmitter() {
    std::cout << "ParticleEmitter destructor" << std::endl;

    glDeleteVertexArrays(1, &VAO_);
    glDeleteBuffers(1, &VBO_);
}

void ParticleEmitter::update(float delta,
                             const Ball& object,
                             int newParticles,
                             const glm::vec2 offset) {
    // Add new particles
    for (GLuint i = 0; i < newParticles; i++) {
        int unusedParticle = this->getFirstUnusedParticle();
        this->respawnParticle(particles_[unusedParticle], object, offset);
    }

    // Update all particles
    for (auto& particle: particles_) {
        particle.lifespan -= delta;
        if (particle.lifespan > 0.0f) {
            particle.position -= particle.velocity * delta;
            particle.color.a -= delta * 2.5;
        }
    }
}

void ParticleEmitter::render() {
    // Use additive blending to give it a 'glow' effect
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    shaderProgram_->use();
    for (auto& particle: particles_) {
        if (particle.lifespan > 0.0f) {
            shaderProgram_->setUniform("offset", particle.position);
            shaderProgram_->setUniform("particleColor", particle.color);
            texture_->bind();
            glBindVertexArray(VAO_);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }
    }
    // Reset to default blending mode
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

GLuint ParticleEmitter::getFirstUnusedParticle() {
    // First search from last used particle, this will usually return almost instantly
    for (GLuint i = lastUsedParticle_; i < amount_; i++) {
        if (particles_[i].lifespan <= 0.0f) {
            lastUsedParticle_ = i;
            return i;
        }
    }

    // Otherwise, do a linear search
    for (GLuint i = 0; i < lastUsedParticle_; i++) {
        if (particles_[i].lifespan <= 0.0f) {
            lastUsedParticle_ = i;
            return i;
        }
    }

    // All particles are taken, override the first one
    // (note that if it repeatedly hits this case, more particles should be reserved)
    lastUsedParticle_ = 0;
    return lastUsedParticle_;
}

void ParticleEmitter::respawnParticle(Particle& particle,
                                      const Ball& object,
                                      const glm::vec2& offset) {
    GLfloat random = ((rand() % 100) - 50) / 10.0f;
    GLfloat color = (GLfloat) 0.5 + ((rand() % 100) / 100.0f);
    particle.position = object.position() + random + offset;
    particle.color = glm::vec4(color, color, color, 1.0f);
    particle.lifespan = 1.0f;
    particle.velocity = object.velocity() * 0.1f;
}

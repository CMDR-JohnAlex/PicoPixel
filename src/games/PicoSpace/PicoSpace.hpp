#pragma once

#include "games/game.hpp"
#include "drivers/potentiometer/b10k.hpp"
#include "utils/math.hpp"

namespace PicoPixel
{
    namespace Games
    {
        // Space dust particle
        struct Particle
        {
            Utils::Vec3 Position;
        };

        class PicoSpace : public Game
        {
        public:
            PicoSpace(Driver::Buffer* buffer);
            virtual ~PicoSpace() = default;

            void OnInit() override;
            void OnShutdown() override;
            bool OnUpdate(float dt) override;
            void OnRender() override;

            std::string GetName() override;
            std::string GetDescription() override;

        private:
            void InitializeParticles();
            void DistributeParticle(Particle* particle);
            void UpdateParticles(float dt);
            void RenderParticles();

            bool Project3DTo2D(const Utils::Vec3& point3D, uint16_t& x, uint16_t& y);

            void PrintMemoryUsage();

        private:
            const uint8_t CROSSHAIR_SIZE = 16;

            const float NEAR_PLANE = 0.1f;
            const float FAR_PLANE = 10000.0f; // Can optionally be 0.0f for no far plane limit.

            // Space Dust
            const uint8_t MAX_PARTICLES = 255;
            const uint8_t PARTICLE_BRIGHTNESS = 200;
            Particle* Particles;

            // Input
            B10kDriver::B10kData* paddle1Potentiometer;
        };
    }
}

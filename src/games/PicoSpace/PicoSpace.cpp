#include "PicoSpace.hpp"
#include "log.hpp"
#include "utils/random.hpp"

#include "pico/stdlib.h"
#include <malloc.h>

namespace PicoPixel
{
    namespace Games
    {
        PicoSpace::PicoSpace(Driver::Buffer* buffer)
         : Game(buffer)
        {
        }

        void PicoSpace::OnInit()
        {
            LOG("PicoSpace: Initializing game\n");

            Particles = new Particle[MAX_PARTICLES];
            LOG("PicoSpace: Allocated %d particles on heap\n", MAX_PARTICLES);

            Potentiometer = new B10kDriver::B10kData();
            B10kDriver::InitializeB10k(Potentiometer, 28, 2);

            InitializeParticles();
        }

        void PicoSpace::OnShutdown()
        {
            LOG("PicoSpace: Shutting down game\n");


            delete[] Particles;
            LOG("PicoSpace: Particle array deallocated\n");

            delete Potentiometer;

            LOG("PicoSpace: Goodbye\n");
        }

        bool PicoSpace::OnUpdate(float dt)
        {
            UpdateParticles(dt);

            // We don't wanna quit the game.
            return false;
        }

        void PicoSpace::OnRender()
        {
            Graphics::FillBuffer(Buffer, 0x0000);

            RenderParticles();

#if 1
            // Lasers! (TEMP/silly implementation)
            // Some modulation should be added to their end (maybe start?) positions to make it not appear completely static.
            // Also, it may benefit from being thicker than 1 pixel.
            static uint8_t L = 0;
            if (L % 16 == 0)
            {
                Graphics::DrawLine(Buffer, 0, Buffer->Height * 0.75, Buffer->Width / 2, Buffer->Height / 2, Utils::RGBto16bit(255, 0, 0));
                Graphics::DrawLine(Buffer, Buffer->Width - 1, Buffer->Height * 0.75, Buffer->Width / 2, Buffer->Height / 2, Utils::RGBto16bit(255, 0, 0));
            }
            L++;
#endif

            // Crosshair
            uint16_t x = Buffer->Width / 2;
            uint16_t y = Buffer->Height / 2;
            Graphics::DrawLine(Buffer, x - CROSSHAIR_SIZE / 2, y, x + CROSSHAIR_SIZE / 2, y, 0xFFFF);
            Graphics::DrawLine(Buffer, x, y - CROSSHAIR_SIZE / 2, x, y + CROSSHAIR_SIZE / 2, 0xFFFF);
        }

        std::string PicoSpace::GetName()
        {
            return "PicoSpace";
        }

        std::string PicoSpace::GetDescription()
        {
            return "Spaceee";
        }

        void PicoSpace::InitializeParticles()
        {
            LOG("PicoSpace: Distributing %d particles in 3D space\n", MAX_PARTICLES);

            for (uint8_t i = 0; i < MAX_PARTICLES; i++)
            {
                DistributeParticle(&Particles[i]);
            }

            LOG("PicoSpace: All particles positioned\n");
        }

        void PicoSpace::DistributeParticle(Particle* particle)
        {
            // The goal is to distribute particles in a sphere around the camera.
            // This will be done by calculating a random distance, rotation, and elevation.
            // (I believe there are special names for these three things but I don't know them...)
            const float NearestParticle = 10.0f;
            const float FarthestParticle = 1000.0f;

            float distance = NearestParticle + (Utils::Rand() % (uint16_t)(FarthestParticle - NearestParticle));
            float rotation = (Utils::Rand() % (2 * 314)) / 100.0f; // Between 0 and 2*PI (full circle)
            float elevation = (Utils::Rand() % 314) / 100.0f; // Between 0 and PI (semi circle)

            // Use 314/100.0f = 3.14 because modulo requires integers, not floats. Also Rand() uses uint16_t and not floats.
            // (fmodf() would work for floats and give more precision, but current method is *probably* faster.)

            particle->Position = Utils::Vec3(
                distance * sinf(rotation) * cosf(elevation),
                distance * sinf(rotation) * sinf(elevation),
                distance * cosf(rotation)
            );

            //LOG("PicoSpace: Particle positioned at (%.1f, %.1f, %.1f) distance=%.1f rotation=%.2f elevation=%.2f\n", particle->Position.x, particle->Position.y, particle->Position.z, distance, rotation, elevation);
        }

        void PicoSpace::UpdateParticles(float dt)
        {
            // FIXME: TEMPORARY INPUT!!
            // Potentiometer mapping
            static float potMin = 64.0f;   // Minimum mapped value (max backward)
            static float potMax = 512.0f;   // Maximum mapped value (max forward)
            static float potStop = 128.0f;  // Center/stop value

            int raw = B10kDriver::ReadB10k(Potentiometer);
            float pot = potMin + ((float)raw / 4095.0f) * (potMax - potMin);
            float speed = pot - potStop;
            for (uint8_t i = 0; i < MAX_PARTICLES; i++)
            {
                Particles[i].Position.z += -speed * dt;

                // Redistribute out-of-range particles.
                const float FarthestParticle = 1000.0f;
                if (Particles[i].Position > FarthestParticle)
                {
                    DistributeParticle(&Particles[i]);
                }
            }
        }

        void PicoSpace::RenderParticles()
        {
            uint8_t visibleCount = 0;

            for (uint8_t i = 0; i < MAX_PARTICLES; i++)
            {
                uint16_t x;
                uint16_t y;
                if (Project3DTo2D(Particles[i].Position, x, y)) // True if point is on screen. If so, draw.
                {
                    visibleCount++;
                    Graphics::DrawPixel(Buffer, x, y, Utils::RGBAto16bit(255, 255, 255, PARTICLE_BRIGHTNESS));
                }
            }

            // Log occasionally (to avoid spam)
            static uint8_t frameCount = 0;
            if (frameCount % (60 * 5) == 0)
            {
                LOG("PicoSpace: Rendered %d/%d visible particles\n", visibleCount, MAX_PARTICLES);
                PrintMemoryUsage();
            }
            frameCount++;
        }

        bool PicoSpace::Project3DTo2D(const Utils::Vec3 &point3D, uint16_t& x, uint16_t& y)
        {
            // Convert 3D coordinates to 2D screen coordinates

            // If point is too close, mark it as off-screen
            if (point3D.z < NEAR_PLANE)
                return false;

            // If point is too far, mark it as off-screen
            if (FAR_PLANE != 0.0f && point3D.z > FAR_PLANE)
                return false;

            // Perspective projection formula:
            // screenX = (worldX / worldZ) * (screenWidth / 2) + (screenWidth / 2)
            // screenY = (worldY / worldZ) * (screenHeight / 2) + (screenHeight / 2)

            // Instead of doing (worldX / worldZ) and (worldY / worldZ) though, we can
            // calculate (1 / worldZ) (the inverse/reciprocal of the Z coordinate)
            // once and multiply worldX and worldY with that since division is slow.
            // (worldX / worldZ) == (worldX * (1 / worldZ))

            // We'll also correct for aspect ratio by scaling X by the same factor as Y.
            // The focal length is (Buffer->Height / 2.0f) and X is multiplied by the aspect ratio (width / height).

            float aspect = (float)Buffer->Width / (float)Buffer->Height;
            float inverseZ = 1.0f / point3D.z;
            float screenX = (point3D.x * inverseZ) * (Buffer->Height / 2.0f) * aspect + Buffer->Width / 2.0f;
            float screenY = (point3D.y * inverseZ) * (Buffer->Height / 2.0f) + Buffer->Height / 2.0f;

            // Check that the projected coordinates are actually on screen.
            if ((int16_t)screenX < 0 || (int16_t)screenX >= (int16_t)Buffer->Width || (int16_t)screenY < 0 || (int16_t)screenY >= (int16_t)Buffer->Height)
                return false;

            x = (uint16_t)screenX;
            y = (uint16_t)screenY;
            return true;
        }

        void PicoSpace::PrintMemoryUsage()
        {
            struct mallinfo mi = mallinfo();
            LOG("Heap total: %d bytes\n", mi.arena);
            LOG("Heap used: %d bytes\n", mi.uordblks);
            LOG("Heap free: %d bytes\n", mi.fordblks);
            LOG("Vec3 size: %d bytes\n", sizeof(Utils::Vec3));
            LOG("Particle size: %d bytes\n", sizeof(Particle));
            LOG("Particles[] size: %d bytes\n", (int)(MAX_PARTICLES * sizeof(Particle)));
            LOG("Buffer struct size: %d bytes\n", (int)sizeof(Driver::Buffer));
            if (Buffer && Buffer->Data)
                LOG("Buffer pixel data size: %d bytes\n", (int)(Buffer->Width * Buffer->Height * sizeof(uint16_t)));
        }
    }
}

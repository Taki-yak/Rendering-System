#include <glm/glm.hpp>
#include <cmath>


class OscillatorComponent : public Component
{
public:
    float amplitude;   
    float speed;       
    float baseY = 0.0f;
    float time = 0.0f;

    OscillatorComponent(float amplitude = 1.0f, float speed = 1.0f)
        : amplitude(amplitude), speed(speed)
    {}

    void OnAttach() override
    {
        baseY = owner->transform.position.y;
    }

    void Update(float deltaTime) override
    {
        time += deltaTime;
        owner->transform.position.y = baseY + std::sin(time * speed) * amplitude;
    }
};
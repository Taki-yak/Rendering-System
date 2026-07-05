#pragma once

#include <string>
#include <map>
#include <iostream>

#include "AnimatedModel.h"

class AnimationLibrary
{
public:
    std::map<std::string, AnimatedModel*> animations;

    void LoadAnimation(
        const std::string& name,
        const std::string& path
    )
    {
        animations[name] =
            new AnimatedModel(path);

        std::cout << "Loaded animation clip: "
            << name
            << std::endl;
    }

    AnimatedModel* GetAnimation(
        const std::string& name
    )
    {
        if (animations.find(name) != animations.end())
        {
            return animations[name];
        }

        return nullptr;
    }
};
#pragma once
#include "Script.h"
#include "Macros.h"

GENERATE_BODY(TrailerBoss);
class TrailerBoss : public Script
{
    FRIEND(TrailerBoss)
public:
    TrailerBoss(GameObject* owner) : Script(owner) {}
    ~TrailerBoss() {}
    void Start() override;
    void Update() override;

private:
    float mSpeed = 0.002f;
    float mMaxSpeed = 0.11f;
    float mYawAngle = 30.0f;
    float mPitchAngle = 270.0f;
    float mDistanceToPlayer = 11.0f;
    float mVerticalOffset = 3.0f;

    GameObject* mFollowTarget = nullptr;

};


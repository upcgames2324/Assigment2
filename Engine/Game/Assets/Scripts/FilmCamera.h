#pragma once
#include "Script.h"
#include "Macros.h"
#include "TimerScript.h"

class CameraComponent;
class SpotLightComponent;

GENERATE_BODY(FilmCamera);
class FilmCamera :public Script
{
    FRIEND(FilmCamera)

public:
    FilmCamera(GameObject* owner) : Script(owner) {}
    void Start() override;
    void Update() override;

private:
    GameObject* mStartPosition = nullptr;
    GameObject* mMidPosition = nullptr;
    GameObject* mEndPosition = nullptr;
    float mSpeed = 0.5f;
    float mRotSpeed = 0.5f;
    float mAngle = 90.0f;
    float mCurrRotation = 0.0f;
    float mRadius = 10.0f;
    float3 mLookAtTarget;
    float mColorSpeed = 0.0f;
    float3 hsv;
    //float density = 0.0f;//0.017
    //float densitySpeed;
    //float heightFallofSpeed;
    //float heightFallof = 0.0f;//0.295
    GameObject* mLightObject = nullptr;
    SpotLightComponent* comp;

    bool mStart = false;
    bool reachedMid = false;
};


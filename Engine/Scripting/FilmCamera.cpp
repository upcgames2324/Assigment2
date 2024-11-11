#include "FilmCamera.h"
#include "GameObject.h"
#include <MathFunc.h>
#include "Application.h"
#include "ModuleInput.h"
#include "ModuleCamera.h"
#include "CameraComponent.h"
#include "Keys.h"
#include "ModuleOpenGL.h"
#include "SpotLightComponent.h"


CREATE(FilmCamera)
{
    CLASS(owner);
    MEMBER(MemberType::GAMEOBJECT, mStartPosition);
    MEMBER(MemberType::GAMEOBJECT, mMidPosition);
    MEMBER(MemberType::GAMEOBJECT, mEndPosition);
    MEMBER(MemberType::FLOAT, mSpeed);
    MEMBER(MemberType::FLOAT, mRotSpeed);
    MEMBER(MemberType::FLOAT, mAngle);
    MEMBER(MemberType::FLOAT, mRadius);
    MEMBER(MemberType::GAMEOBJECT, mLightObject);
    MEMBER(MemberType::FLOAT, mColorSpeed);
    //MEMBER(MemberType::FLOAT, densitySpeed);
    //MEMBER(MemberType::FLOAT, heightFallofSpeed);
    END_CREATE;
}

void HSVtoRGB(float& fR, float& fG, float& fB, float& fH, float& fS, float& fV) {
    float fC = fV * fS; // Chroma
    float fHPrime = fmod(fH / 60.0, 6);
    float fX = fC * (1 - fabs(fmod(fHPrime, 2) - 1));
    float fM = fV - fC;

    if (0 <= fHPrime && fHPrime < 1) {
        fR = fC;
        fG = fX;
        fB = 0;
    }
    else if (1 <= fHPrime && fHPrime < 2) {
        fR = fX;
        fG = fC;
        fB = 0;
    }
    else if (2 <= fHPrime && fHPrime < 3) {
        fR = 0;
        fG = fC;
        fB = fX;
    }
    else if (3 <= fHPrime && fHPrime < 4) {
        fR = 0;
        fG = fX;
        fB = fC;
    }
    else if (4 <= fHPrime && fHPrime < 5) {
        fR = fX;
        fG = 0;
        fB = fC;
    }
    else if (5 <= fHPrime && fHPrime < 6) {
        fR = fC;
        fG = 0;
        fB = fX;
    }
    else {
        fR = 0;
        fG = 0;
        fB = 0;
    }

    fR += fM;
    fG += fM;
    fB += fM;
}

void FilmCamera::Start()
{
    //const std::vector<GameObject*> children = mGameObject->GetChildren();
    //if (!children.empty()) mCameraObject = mGameObject->GetChildren()[0];
    //
    //mGameObject->SetWorldPosition(mFollowTarget->GetWorldPosition());
    //mGameObject->SetWorldRotation(float3(DegToRad(mYawAngle), DegToRad(mPitchAngle), 0.0f));
    //mGameObject->Translate(-(mGameObject->GetFront().Add(mOffset)) * mDistanceToTarget);

    mGameObject->SetWorldPosition(mStartPosition->GetWorldPosition());
    mGameObject->SetWorldRotation(mStartPosition->GetWorldRotation());
    //mGameObject->Translate(mGameObject->GetFront().Mul(mRadius));
    //mLookAtTarget = mGameObject->GetWorldPosition();
    //mGameObject->Translate(-mGameObject->GetFront().Mul(mRadius));

    hsv = float3(0.f, 163.f/255.f, 255.f/255.f);
    comp = static_cast<SpotLightComponent*>(mLightObject->GetComponent(ComponentType::SPOTLIGHT));
    comp->SetColor(hsv.ptr());
    //App->GetOpenGL()->SetBloomIntensity(0.0f);
}

void FilmCamera::Update()
{
    //mGameObject->SetWorldPosition(mFollowTarget->GetWorldPosition());
    ////mGameObject->SetWorldRotation(float3(DegToRad(mYawAngle), DegToRad(mPitchAngle), 0.0f));
    //mGameObject->Translate(-(mGameObject->GetFront().Add(mOffset))*mDistanceToTarget);
    //
    //if (!mShakeTimer.Delay(mShakeDuration))
    //{
    //    Shake();
    //}
    //else
    //{
    //    if (mCameraObject) mCameraObject->SetLocalPosition(float3::zero);
    //    mShakeDuration = 0.0f;
    //}
    //float3 dir = (mEndPosition->GetWorldPosition() - mGameObject->GetWorldPosition()).Normalized();
    
    //static bool bloomToggle = false;
    //if (App->GetInput()->GetKey(Keys::Keys_J) == KeyState::KEY_DOWN)
    //{
    //        mBloomSpeed = -mBloomSpeed;
    //        bloomToggle = true;
    //}
    //if (bloomToggle)
    //{
    //    float newIntensity = App->GetOpenGL()->GetBloomIntensity() + mBloomSpeed;
    //    if(newIntensity < 1.0f && newIntensity > 0.0f)
    //        App->GetOpenGL()->SetBloomIntensity(newIntensity);
    //    else if (newIntensity < 0.0f)
    //    {
    //        App->GetOpenGL()->SetBloomIntensity(0.0f);
    //        bloomToggle = false;
    //    }
    //    else if (newIntensity > 1.0f)
    //    {
    //        App->GetOpenGL()->SetBloomIntensity(1.0f);
    //        bloomToggle = false;
    //    }
    //}
    //if (App->GetInput()->GetKey(Keys::Keys_J) == KeyState::KEY_DOWN)
    //    App->GetOpenGL()->ToggleAO();
    //static bool densityToggle = false;
    //if (App->GetInput()->GetKey(Keys::Keys_U) == KeyState::KEY_DOWN)
    //    densityToggle = !densityToggle;
    //if (densityToggle && density < 0.17f)
    //{
    //    density += 0.00005f;
    //    App->GetOpenGL()->SetFogDensity(density);
    //}
    //static bool heightFallofToggle = false;
    //if (App->GetInput()->GetKey(Keys::Keys_J) == KeyState::KEY_DOWN)
    //    heightFallofToggle = !heightFallofToggle;
    //if (heightFallofToggle && heightFallof < 0.5f)
    //{
    //    heightFallof += 0.0005f;
    //    App->GetOpenGL()->SetFogHeightFallof(heightFallof);
    //}
    static bool colorToggle = false;
    if (App->GetInput()->GetKey(Keys::Keys_M) == KeyState::KEY_DOWN)
        colorToggle = !colorToggle;
    if (colorToggle)
    {
        float col[3];
        hsv.x += mColorSpeed;
        float hsvX = static_cast<int>(hsv.x) % 360;
        HSVtoRGB(col[0], col[1], col[2], hsvX, hsv.y, hsv.z);
        comp->SetColor(col);
    }
    if (App->GetInput()->GetKey(Keys::Keys_H) == KeyState::KEY_DOWN)
        mStart = !mStart;
    if (mStart && mGameObject->GetWorldPosition().Distance(mEndPosition->GetWorldPosition()) > 0.1f)
    {
        static float3 dir = (mEndPosition->GetWorldPosition() - mStartPosition->GetWorldPosition()).Normalized();
        mGameObject->Translate(dir.Mul(mSpeed * App->GetDt()));
    }
    //if (!reachedMid && mGameObject->GetWorldPosition().Distance(mMidPosition->GetWorldPosition()) < 0.1f)
    //    reachedMid = true;
    //if (reachedMid && (mCurrRotation < mAngle))
    //{
    //    Quat rotation = Quat::RotateAxisAngle(mStartPosition->GetRight(), DegToRad(mCurrRotation));
    //    mGameObject->SetWorldRotation(rotation * mStartPosition->GetWorldRotation());
    //    mCurrRotation += mRotSpeed;
    //}
    //if (mStart)
    //{
    //    if(mGameObject->GetWorldPosition().Distance(mEndPosition->GetWorldPosition()) > 0.1f)
    //        mGameObject->Translate(mGameObject->GetRight().Mul(mSpeed*App->GetDt()));
    //    else
    //    {
    //        float3 tmp = mEndPosition->GetWorldPosition();
    //        mEndPosition->SetWorldPosition(mStartPosition->GetWorldPosition());
    //        mStartPosition->SetWorldPosition(tmp);
    //        mSpeed = -mSpeed;
    //    }
    //    //mGameObject->LookAt(mEndPosition->GetWorldPosition());
    //}
    //if (mStart && (mCurrRotation < mAngle))
    //if (mStart)
    //{
    //    if ((abs(mCurrRotation) < mAngle))
    //    {
    //        Quat rotation = Quat::RotateAxisAngle(mStartPosition->GetUp(), DegToRad(mCurrRotation));
    //        mGameObject->SetWorldRotation(rotation * mStartPosition->GetWorldRotation());
    //        mCurrRotation += mSpeed;
    //    }
    //    else
    //    {
    //        mStartPosition->SetWorldRotation(Quat::RotateAxisAngle(mStartPosition->GetUp(), DegToRad(mAngle)) * mStartPosition->GetWorldRotation());
    //        mSpeed = -mSpeed;
    //        mCurrRotation = 0.0f;
    //    }
    //}
    //if (mStart)
    //{
    //    mGameObject->Translate(mGameObject->GetRight().Mul(mSpeed*App->GetDt()));
    //    mGameObject->LookAt(mLookAtTarget);
    //    //float3x3 rotation = float3x3::RotateAxisAngle(mStartPosition->GetUp(), DegToRad(mCurrRotation));
    //    //float3x3 rotation2 = float3x3::RotateAxisAngle(mStartPosition->GetRight(), DegToRad(mCurrRotation));
    //    //mGameObject->SetWorldRotation(Quat(rotation) * mStartPosition->GetWorldRotation());
    //    //mCurrRotation += mSpeed;
    //    //mGameObject->Translate(-mGameObject->GetFront().Mul(mRadius));
    //}
    //if (mStart)
    //{
    //    mGameObject->Translate(mGameObject->GetFront().Mul(mRadius));
    //    float3x3 rotation = float3x3::RotateAxisAngle(mStartPosition->GetUp(), DegToRad(mCurrRotation));
    //    float3x3 rotation2 = float3x3::RotateAxisAngle(mStartPosition->GetRight(), DegToRad(mCurrRotation));
    //    mGameObject->SetWorldRotation(Quat(rotation) * mStartPosition->GetWorldRotation());
    //    mCurrRotation += mSpeed;
    //    mGameObject->Translate(-mGameObject->GetFront().Mul(mRadius));
    //}
    //if (mStart)
    //{
    //    mGameObject->Translate(-mGameObject->GetFront().Mul(mRadius));
    //    float3x3 rotation = float3x3::RotateAxisAngle(mStartPosition->GetUp(), DegToRad(mCurrRotation));
    //    mGameObject->SetWorldRotation(Quat(rotation) * mStartPosition->GetWorldRotation());
    //    mCurrRotation += mSpeed;
    //    mGameObject->Translate(mGameObject->GetFront().Mul(mRadius));
    //}

    //float3x3 rotation = float3x3::RotateAxisAngle(mStartPosition->GetRight(), DegToRad(mCurrRotation));
}
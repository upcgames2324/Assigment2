#include "TrailerBoss.h"
#include "GameObject.h"
#include "AnimationComponent.h"
#include <MathFunc.h>
#include "Application.h"

CREATE(TrailerBoss)
{
    CLASS(owner);
    SEPARATOR("TRAILER CONTENT SCRIPT");
    MEMBER(MemberType::GAMEOBJECT, mFollowTarget);
    MEMBER(MemberType::FLOAT, mSpeed);
    MEMBER(MemberType::FLOAT, mMaxSpeed);
    MEMBER(MemberType::FLOAT, mYawAngle);
    MEMBER(MemberType::FLOAT, mPitchAngle);
    MEMBER(MemberType::FLOAT, mDistanceToPlayer);
    MEMBER(MemberType::FLOAT, mVerticalOffset);


    END_CREATE;
}

void TrailerBoss::Start()
{
    const std::vector<GameObject*> children = mGameObject->GetChildren();

    mGameObject->SetWorldPosition(mFollowTarget->GetWorldPosition());
    mGameObject->SetWorldRotation(float3(DegToRad(mYawAngle), DegToRad(mPitchAngle), 0.0f));
    mGameObject->Translate(-(mGameObject->GetFront()) * mDistanceToPlayer);
    AnimationComponent* anim = static_cast<AnimationComponent*>(mFollowTarget->GetComponent(ComponentType::ANIMATION));
    if (anim)
    {
        anim->SetIsPlaying(true);
        anim->SetControllerTime(0.02f);
    }
}

void TrailerBoss::Update()
{
    mDistanceToPlayer += mSpeed;
    mGameObject->SetWorldPosition(mFollowTarget->GetWorldPosition());
    //mGameObject->SetWorldRotation(float3(DegToRad(mYawAngle), DegToRad(mPitchAngle), 0.0f));
    mGameObject->Translate(-(mGameObject->GetFront()) * mDistanceToPlayer);
    mGameObject->Translate((mGameObject->GetUp()) * mVerticalOffset);
    if(mSpeed<mMaxSpeed) mSpeed += App->GetDt() *mSpeed * (2.0f/3.0f);

}

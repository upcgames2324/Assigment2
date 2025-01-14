#include "EnemyBoss.h"
#include "Application.h"
#include "GameObject.h"
#include "ModuleScene.h"
#include "AIAGentComponent.h"
#include "AnimationComponent.h"
#include "MeshRendererComponent.h"
#include "ScriptComponent.h"
#include "GameManager.h"
#include "AudioManager.h"
#include "PoolManager.h"
#include "BombBoss.h"
#include "ColorGradient.h"
#include "Bullet.h"
#include "BossLaser.h"
#include "HudController.h"
#include "MathFunc.h"
#include "BossBattleArea.h"
#include "ImageComponent.h"
#include <algorithm>
#include "ModuleResource.h"


#define LASER_WIND_UP 2.625f
#define BULLETS_ANIMATION 146.0f / 24.0f
#define BULLETS_WIND_UP 92.0f / 24.0f
#define ERUPTION_ANIMATION 144.0f / 24.0f
#define IDLE_ANIMATION 40.0f / 24.0f
#define PHASE_ANIMATION 5.0f
#define DEATH_ANIMATION 4.4583f
#define WAKEUP_ANIMATION 5.0f
#define DEFENSE_START_ANIMATION 1.791f
#define DEFENSE_END_ANIMATION 1.2f
#define HIT_ANIMATION 1.25f
#define BEAT_TIME 0.428571435f
#define ATTACK_CONE_RADIANTS 4.1f

CREATE(EnemyBoss) {
    CLASS(owner);
    SEPARATOR("STATS");
    MEMBER(MemberType::FLOAT, mMaxHealth);
    MEMBER(MemberType::FLOAT, mAttackDistance);
    MEMBER(MemberType::FLOAT, mAttackCoolDown);
    MEMBER(MemberType::FLOAT, mAttackSequenceCooldown);
    MEMBER(MemberType::FLOAT, mPhaseShiftTime);
    MEMBER(MemberType::FLOAT, mPhase1Hp);
    MEMBER(MemberType::FLOAT, mPhase2Hp);
    MEMBER(MemberType::FLOAT, mSpawnOffsetY);
    MEMBER(MemberType::FLOAT, mSpeed);
    MEMBER(MemberType::GAMEOBJECT, mShieldGO);
    SEPARATOR("AREA POSITIONS");
    MEMBER(MemberType::GAMEOBJECT, mAreas[0]);
    MEMBER(MemberType::GAMEOBJECT, mAreas[1]);
    MEMBER(MemberType::GAMEOBJECT, mAreas[2]);
    SEPARATOR("BULLET HELL");
    MEMBER(MemberType::FLOAT, mBulletHellDuration);
    MEMBER(MemberType::FLOAT, mBulletSpeed);
    MEMBER(MemberType::FLOAT, mBulletRange);
    MEMBER(MemberType::FLOAT, mBulletsDamage);
    SEPARATOR("BOMBS");
    MEMBER(MemberType::FLOAT, mBombsDuration);
    MEMBER(MemberType::FLOAT, mBombsDelay);
    MEMBER(MemberType::FLOAT, mBombDamage);
    SEPARATOR("LASER");
    MEMBER(MemberType::GAMEOBJECT, mLaserGO);
    MEMBER(MemberType::FLOAT, mLaserDamage);
    MEMBER(MemberType::FLOAT, mLaserSpeed);
    MEMBER(MemberType::FLOAT, mLaserDuration);
    SEPARATOR("LASER");
    MEMBER(MemberType::GAMEOBJECT, mDeathParticlesHead);
    MEMBER(MemberType::GAMEOBJECT, mDeathParticlesBody);
    MEMBER(MemberType::GAMEOBJECT, mDeathParticlesLegs);
    END_CREATE;
}

EnemyBoss::EnemyBoss(GameObject* owner) : Enemy(owner)
{
}

void EnemyBoss::Start()
{
    Enemy::Start();
    mCurrentState = EnemyState::DOWN;
    mFront = mGameObject->GetFront();
    mRotationSpeed = 0.0f;
    mOgPosition = mGameObject->GetWorldPosition();
    mGameObject->SetWorldPosition(mOgPosition + float3::unitY * mSpawnOffsetY);

    for (const char* prefab : mTemplateNames)
    {
		for (int i = 0; i < 3; i++)
		{
            GameObject* bombTemplate = App->GetScene()->InstantiatePrefab(prefab, mGameObject->GetParent());
            if (bombTemplate)
            {
                bombTemplate->SetEnabled(false);
                mTemplates.push_back(bombTemplate);
            }
		}
    }

    mAnimationComponent = static_cast<AnimationComponent*>(mGameObject->GetComponent(ComponentType::ANIMATION));
    if (mAnimationComponent)
    {
        mAnimationComponent->SetIsPlaying(true);
        mAnimationComponent->SetAnimSpeed(0.5f);
    }

    for (int i = 0; i < std::size(mAreas); ++i) 
    {
        if (mAreas[i]) 
        {
            mAreaPositions.push_back(mAreas[i]->GetWorldPosition());
        }
    }

    if (mShieldGO)
    {
        mSpritesheet = static_cast<ImageComponent*>(mShieldGO->GetComponent(ComponentType::IMAGE));
    }
    mHitEffectColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
    mDeathParticlesHead->SetEnabled(false);
    mDeathParticlesBody->SetEnabled(false);
    mDeathParticlesLegs->SetEnabled(false);
}

void EnemyBoss::Update()
{
    if (GameManager::GetInstance()->IsPaused()) return;
    if (GameManager::GetInstance()->GetHud()) GameManager::GetInstance()->GetHud()->SetBossHealth(mHealth / mMaxHealth);
    float t = HIT_ANIMATION;
    static short phaseChange = 0;
    static bool scream = true;
    Rotate();

    if ((mStage == 1 && mHealth / mMaxHealth < mPhase2Hp) || (mStage == 0 && mHealth / mMaxHealth < mPhase1Hp))
    {
        //Phase change
        phaseChange = 0;
        ++mStage;
        if (mStage == 1)
        {
            mHealth = mMaxHealth * mPhase1Hp;
            SwitchMaterial(mPhase2MatId);
        }
        else if (mStage == 2)
        {
            mHealth = mMaxHealth * mPhase2Hp;
            SwitchMaterial(mPhase3MatId);
        }
        mCurrentState = EnemyState::PHASE;
        InterruptAttacks();
        mInvulnerable = true;
        GameManager::GetInstance()->GetHud()->SetBossInvulnerable(mInvulnerable);
        if (mAnimationComponent) mAnimationComponent->SendTrigger("tHit1", mDeathTransitionDuration);
        
        return;
    }

   

    if (!mBeAttracted)
    {
        switch (mStage)
        {
        case 0:
            UpdatePhase1();
            break;
        case 1:
            UpdatePhase2();
            break;
        case 2:
            UpdatePhase3();
            break;
        }

        switch (mCurrentState)
        {
        case EnemyState::DOWN:
            if (mWakeUp)
            {
                mCurrentState = EnemyState::WAKE;                
            }
            break;
        case EnemyState::WAKE:
            if (!mGameObject->GetWorldPosition().Equals(mOgPosition))
            {
                float dt = App->GetDt() * mSpeed;
                if (dt * dt >= mOgPosition.DistanceSq(mGameObject->GetWorldPosition()))
                {
                    mGameObject->SetWorldPosition(mOgPosition);                    
                }
                else
                {
                    float3 newPos = mGameObject->GetWorldPosition() + (mOgPosition - mGameObject->GetWorldPosition()).Normalized() * dt;
                    mGameObject->SetWorldPosition(newPos);
                }
            }
            if (mChargeDurationTimer.Delay(WAKEUP_ANIMATION * 0.3) && scream)
            {
                scream = false;
                GameManager::GetInstance()->GetAudio()->PlayOneShot(SFX::BOSS_SCREAM, GameManager::GetInstance()->GetPlayer()->GetWorldPosition());
            }
            if (mPhaseShiftTimer.Delay(WAKEUP_ANIMATION))
            {
                GameManager::GetInstance()->GetHud()->SetBossHealthBarEnabled(true);
                GameManager::GetInstance()->SetIsFightingBoss(true);
                mAnimationComponent->SetAnimSpeed(1.0f);
                mWakeUp = false;
                mInvulnerable = false;
                GameManager::GetInstance()->GetHud()->SetBossInvulnerable(mInvulnerable);
                mCurrentState = EnemyState::IDLE;
                mChargeDurationTimer.Reset();
                if (mAnimationComponent) mAnimationComponent->SendTrigger("tIdle", mDeathTransitionDuration);
            }
            break;
        case EnemyState::PHASE:
            
            switch (phaseChange)
            {
            case 0:
                if (mPhaseShiftTimer.Delay(HIT_ANIMATION))
                {
                    if (mAnimationComponent) mAnimationComponent->SendTrigger("tDefenseStart", mDeathTransitionDuration);
                    ++phaseChange;
                    GameManager::GetInstance()->GetAudio()->PlayOneShot(SFX::BOSS_SCREAM, GameManager::GetInstance()->GetPlayer()->GetWorldPosition());
                    if (mSpritesheet)
                    {
                        mShieldGO->SetEnabled(true);
                        mSpritesheet->PlayAnimation();
                        mShieldDelay = 18.0f / mSpritesheet->GetFrameDuration();
                        mShieldTimer.Reset();
                    }
                }
                break;
            case 1:
                if (mPhaseShiftTimer.Delay(DEFENSE_START_ANIMATION))
                {
                    BossBattleArea* ba = static_cast<BossBattleArea*>(GameManager::GetInstance()->GetActiveBattleArea());
                    if (ba) ba->SpawnEnemies();
                    if (mAnimationComponent) mAnimationComponent->SendTrigger("tDefenseLoop", mDeathTransitionDuration);
                    ++phaseChange;
                }
                if (mShieldTimer.Delay(mShieldDelay))
                {
                    if (mSpritesheet) mSpritesheet->PauseAnimation();
                }
                break;
            case 2:
                if (mWakeUp)
                {
                    if (mAnimationComponent) mAnimationComponent->SendTrigger("tDefenseEnd", 1.0f);
                    ++phaseChange;
                    mWakeUp = false;
                }
                break;
            case 3:
                if (mPhaseShiftTimer.Delay(DEFENSE_END_ANIMATION))
                {
                    if (mAnimationComponent) 
                    {
                        mAnimationComponent->SendTrigger("tPhase", mDeathTransitionDuration);
                        mAnimationComponent->SetAnimSpeed(PHASE_ANIMATION / mPhaseShiftTime);
                    }
                    ++phaseChange;
                    GameManager::GetInstance()->GetAudio()->PlayOneShot(SFX::BOSS_AWAKE, GameManager::GetInstance()->GetPlayer()->GetWorldPosition());
                    if (mSpritesheet) mSpritesheet->PlayAnimation();
                    mShieldTimer.Reset();
                }
                break;            
            case 4:
            case 5:
                if (mPhaseShiftTimer.Delay(mPhaseShiftTime))
                {
                    if (mAnimationComponent) 
                    {
                        mAnimationComponent->SendTrigger("tIdle", mDeathTransitionDuration);
                        mAnimationComponent->SetAnimSpeed(1.0f);
                    }
                    LookAt(mFront, BEAT_TIME);
                    phaseChange++;
                    
                }
                if (mShieldTimer.Delay(mShieldDelay) && mShieldGO->IsEnabled())
                {
                    if (mSpritesheet)
                    {
                        mSpritesheet->StopAnimation();
                        mShieldGO->SetEnabled(false);
                    }
                    phaseChange++;
                }
                break;
            case 6:
                mInvulnerable = false;
                GameManager::GetInstance()->GetHud()->SetBossInvulnerable(mInvulnerable);
                mPhaseShiftTimer.Reset();
                mCurrentState = EnemyState::IDLE;
                phaseChange = 0;
            }
           
            break;
        case EnemyState::DEATH:
            if (mAnimationComponent) mAnimationComponent->SendTrigger("tDeath", mDeathTransitionDuration);
            InterruptAttacks();
            Death();
            break;
        case EnemyState::CHARGING_BULLET_HELL:
            if (mPhaseShiftTimer.Delay(BULLETS_WIND_UP))
            {
                if (mAnimationComponent) mAnimationComponent->SendTrigger("tBulletHell", mAttackTransitionDuration);
                mCurrentState = EnemyState::ATTACK;
            }
            break;
        case EnemyState::CHARGING_LASER:
            if (mPhaseShiftTimer.Delay(LASER_WIND_UP))
            {
                mCurrentState = EnemyState::ATTACK;
                if (mAnimationComponent) mAnimationComponent->SendTrigger("tLaser", mAttackTransitionDuration);
            }
            break;
        
        }
    }

    if (mBulletHell != BulletPattern::NONE)
    {
        if (mAttackDurationTimer.Delay(mBulletHellDuration)) 
        {
            mBulletHell = BulletPattern::NONE;
            LookAt(mFront, 2 * BEAT_TIME);
        }
        else switch (mBulletHell)
        {
        case BulletPattern::CIRCLES:
            BulletHellPattern1();
            break;
        case BulletPattern::ARROW:
            BulletHellPattern2();
            break;
        case BulletPattern::WAVE:
            BulletHellPattern5();
            break;
        case BulletPattern::TARGETED_CIRCLES:
            BulletHellPattern6();
            break;
        }
    }
        
    CheckHitEffect();
}

void EnemyBoss::StartBulletAttack(BulletPattern pattern)
{
    GameManager::GetInstance()->GetAudio()->PlayOneShot(SFX::BOSS_ROAR_BULLET);
    mCurrentState = EnemyState::CHARGING_LASER;
    if (mAnimationComponent) mAnimationComponent->SendTrigger("tLaserCharge", mAttackTransitionDuration);
    mBulletHellTimer.Reset();
    mBulletHell = pattern;
    mBulletsWave = 0;
    mAttackDurationTimer.Reset();
}

void EnemyBoss::LaserAttack()
{
    GameManager::GetInstance()->GetAudio()->PlayOneShot(SFX::BOSS_ROAR_LASER);
    LookAt(mFront, 2 * BEAT_TIME);
    mCurrentState = EnemyState::CHARGING_LASER;
    if (mAnimationComponent) mAnimationComponent->SendTrigger("tLaserCharge", mAttackTransitionDuration);

    if (mLaserGO)
    {
        float3 laserSpawn = mGameObject->GetWorldPosition();
        laserSpawn.y = mPlayer->GetWorldPosition().y + 2.0f;
        mLaserGO->SetWorldPosition(laserSpawn);
        BossLaser* laserScript = static_cast<BossLaser*>(static_cast<ScriptComponent*>(mLaserGO->GetComponent(ComponentType::SCRIPT))->GetScriptInstance());
        if (laserScript)
        {
            laserScript->Init(mLaserDamage,mLaserDuration,mLaserDistance,mLaserSpeed);
        }
    }
}

void EnemyBoss::BombAttack(const char* pattern)
{
    GameManager::GetInstance()->GetAudio()->PlayOneShot(SFX::BOSS_ROAR_ERUPTION);
    mCurrentState = EnemyState::ATTACK;
    const int n = sizeof(mTemplateNames) / sizeof(mTemplateNames[0]);

    if (mAnimationComponent) mAnimationComponent->SendTrigger("tEruption", mAttackTransitionDuration);
    const char** targetPtr = std::find(&mTemplateNames[0], mTemplateNames + n, pattern);
    int index = targetPtr - mTemplateNames;

    if (index == 2)
    {
        for (int i = 0; i < mAreaPositions.size(); i++)
        {
			float playerAreaDistance = mPlayer->GetWorldPosition().Distance(mAreaPositions[i]);
            mPlayerAreaDistances[playerAreaDistance] = i;
        }

		int playerZone = mPlayerAreaDistances.begin()->second;
		int freeZone = 0;

		//If the player is in the middle, the free are will be the hardest to reach for the player
		if (playerZone == 1) freeZone = mPlayerAreaDistances.rbegin()->second;
		else freeZone = mPlayerAreaDistances.at(std::next(mPlayerAreaDistances.begin(), 1)->first);

        for (int i = 0; i < mAreaPositions.size(); i++)
        {
			if (i == freeZone) continue;

            int templateIndex = index * 3 + i;
            GameObject* bombGO = mTemplates[templateIndex];
            SetupBomb(bombGO, mAreaPositions[i]);
        }
		mPlayerAreaDistances.clear();
    }
    else
    {
        if (index == 1)
        {
            //Spawn 2 single bombs on the area separators
            SetupBomb(mTemplates[10], mAreaPositions[0] + math::float3(2.45f, 0.0f, -4.55f));
            SetupBomb(mTemplates[11], mAreaPositions[2] + math::float3(2.45f, 0.0f, 4.55f));
        }

        SetupBomb(mTemplates[9], mPlayer->GetWorldPosition());

        for (int i = 0; i < mAreaPositions.size(); i++)
        {
            int templateIndex = index * 3 + i;
            SetupBomb(mTemplates[templateIndex], mAreaPositions[i]);
        }
    }
}

void EnemyBoss::SetupBomb(GameObject* bombGO, const math::float3& position)
{
    bombGO->SetWorldPosition(position);
    bombGO->SetEnabled(true);

    std::vector<Component*> scriptComponents;
    bombGO->GetComponentsInChildren(ComponentType::SCRIPT, scriptComponents);
    for (Component* scriptComponent : scriptComponents) {
        BombBoss* bombScript = static_cast<BombBoss*>(static_cast<ScriptComponent*>(scriptComponent)->GetScriptInstance());
        bombScript->Init(mGameObject->GetWorldPosition(), mBombDamage, mBombsDelay);
    }
}

void EnemyBoss::CheckHitEffect()
{
    if (mHit)
    {
        if (mHitEffectTimer.Delay(mHitEffectTime))
        {
            ResetEnemyColor(1.0f);
            mHit = false;
        }
        else
        {
            ResetEnemyColor(mHitEffectTimer.GetTimePassed() / mHitEffectTime);
        }
    }
}

void EnemyBoss::Death()
{
    float factor = mDeathTimer.GetTimePassed() / DEATH_ANIMATION;
    for (size_t i = 0; i < mMeshComponents.size(); i++)
    {
        MeshRendererComponent* meshComponent = static_cast<MeshRendererComponent*>(mMeshComponents[i]);
        meshComponent->SetBaseColorFactor(mOgColors[i] - float4(0.0f, 0.0f, 0.0f, math::Pow(factor, 2)));
    }
    if (mDeathTimer.Delay(DEATH_ANIMATION))
    {
        mDeathParticlesHead->SetEnabled(false);
        mDeathParticlesBody->SetEnabled(false);
        mDeathParticlesLegs->SetEnabled(false);
        //GameManager::GetInstance()->GetHud()->SetBossHealthBarEnabled(false);
        GameManager::GetInstance()->SetIsFightingBoss(false);
        mGameObject->SetEnabled(false);
        GameManager::GetInstance()->Victory();
    }
}

void EnemyBoss::BulletHellPattern1() //Circular
{
    if (mBulletHellTimer.Delay(2 * BEAT_TIME)) //Each pattern will need different rythm
    {
        unsigned int nBullets = 14;
        float alpha = ATTACK_CONE_RADIANTS / (nBullets - 1);
        float offset = 0.0f;
        if (mBulletsWave % 2 == 1)
        {
            offset = alpha / 2;
            nBullets--;
        }
        float3 bulletOriginPosition = mGameObject->GetWorldPosition();
        bulletOriginPosition.y = mPlayer->GetWorldPosition().y + 2.0f;
        float3 rotation = mGameObject->GetWorldEulerAngles();
        for (unsigned int i = 0; i < nBullets; ++i)
        {
            float angle = (-ATTACK_CONE_RADIANTS / 2) + offset + i * alpha;
            GameObject* bulletGO = GameManager::GetInstance()->GetPoolManager()->Spawn(PoolType::BOSS_BULLET);
            bulletGO->SetWorldPosition(bulletOriginPosition);

            float3 direction = float3(mFront.x * cos(angle) - mFront.z * sin(angle), mFront.y, mFront.x * sin(angle) + mFront.z * cos(angle));
            direction.Normalize();
            Bullet* bulletScript = static_cast<Bullet*>(static_cast<ScriptComponent*>(bulletGO->GetComponent(ComponentType::SCRIPT))->GetScriptInstance());
            ColorGradient gradient;
            gradient.AddColorGradientMark(0.1f, float4(255.0f, 255.0f, 255.0f, 1.0f));
            bulletScript->Init(bulletOriginPosition, direction, mBulletSpeed, 1.0f, &gradient, mBulletsDamage, mBulletRange);

            if (i % 2 == 0)
            {
                GameManager::GetInstance()->GetAudio()->PlayOneShot(SFX::ENEMY_ROBOT_GUNFIRE, bulletGO->GetWorldPosition());
            }
        }
        mBulletsWave++;
    }
}

void EnemyBoss::BulletHellPattern2() //Arrow
{
    
    const unsigned int nBullets = 8;
    float delay = 0.25f * BEAT_TIME;
    if (mBulletsWave % nBullets == 0) delay = 2.0f * BEAT_TIME;
    if (mBulletHellTimer.Delay(delay)) //Each pattern will need different rythm
    {
        const float space = 0.3f;
        
        float3 bulletOriginPosition = mGameObject->GetWorldPosition();
        float3 target = mPlayer->GetWorldPosition();
        target.y += 2.0f;
        bulletOriginPosition.y = target.y;
        static float3 direction = (target - bulletOriginPosition).Normalized();
        if (mBulletsWave % nBullets == 0)
        {
            direction = target - bulletOriginPosition;
            direction.Normalize();
            LookAt(direction, 0.25f * BEAT_TIME);
        }
        float3 right = mGameObject->GetUp().Cross(direction);
        
        for (int i : { -1, 1 })
        {
            GameObject* bulletGO = GameManager::GetInstance()->GetPoolManager()->Spawn(PoolType::BOSS_BULLET);
            float3 position = bulletOriginPosition + right * space * (mBulletsWave % nBullets) * i;

            Bullet* bulletScript = static_cast<Bullet*>(static_cast<ScriptComponent*>(bulletGO->GetComponent(ComponentType::SCRIPT))->GetScriptInstance());
            ColorGradient gradient;
            gradient.AddColorGradientMark(0.1f, float4(255.0f, 255.0f, 255.0f, 1.0f));
            bulletScript->Init(position, direction, mBulletSpeed, 1.0f, &gradient, mBulletsDamage, mBulletRange);
        }

        if (mBulletsWave % 2 == 0)
        {
            GameManager::GetInstance()->GetAudio()->PlayOneShot(SFX::ENEMY_ROBOT_GUNFIRE, bulletOriginPosition);
        }
        else 
        {
            GameManager::GetInstance()->GetAudio()->PlayOneShot(SFX::PLAYER_PISTOL, bulletOriginPosition);
        }

        mBulletsWave++;
    }
}

void EnemyBoss::BulletHellPattern3() //Two streams
{
    
    if (mBulletHellTimer.Delay(0.25f * BEAT_TIME)) //Each pattern will need different rythm
    {
        const unsigned int nBullets = 16;
        float alpha = (pi / 2) - pi * (mBulletsWave % nBullets) / (nBullets - 1);
        float3 bulletOriginPosition = mGameObject->GetWorldPosition();
        bulletOriginPosition.y = mPlayer->GetWorldPosition().y + 2.0f;

        for (int i : { -1, 1 })
        {
            GameObject* bulletGO = GameManager::GetInstance()->GetPoolManager()->Spawn(PoolType::BOSS_BULLET);
            float angle = alpha * i;
            float3 direction = float3(mFront.x * cos(angle) - mFront.z * sin(angle), mFront.y, mFront.x * sin(angle) + mFront.z * cos(angle));
            direction.Normalize();
            Bullet* bulletScript = static_cast<Bullet*>(static_cast<ScriptComponent*>(bulletGO->GetComponent(ComponentType::SCRIPT))->GetScriptInstance());
            ColorGradient gradient;
            gradient.AddColorGradientMark(0.1f, float4(255.0f, 255.0f, 255.0f, 1.0f));
            bulletScript->Init(bulletOriginPosition, direction, mBulletSpeed, 1.0f, &gradient, mBulletsDamage, mBulletRange);

            GameManager::GetInstance()->GetAudio()->PlayOneShot(SFX::ENEMY_ROBOT_GUNFIRE, bulletGO->GetWorldPosition());
        }
        mBulletsWave++;
    }
}

void EnemyBoss::BulletHellPattern4() //Curved Arrows
{
    
    const unsigned int nBullets = 8;
    const float width = 2.0f;
    float delay = 0.25f * BEAT_TIME;
    if (mBulletsWave % nBullets == 0) delay = 2.0f * BEAT_TIME;
    if (mBulletHellTimer.Delay(delay)) //Each pattern will need different rythm
    {
        
        float3 bulletOriginPosition = mGameObject->GetWorldPosition();
        float3 target = mPlayer->GetWorldPosition();
        target.y += 2.0f;
        bulletOriginPosition.y = target.y;
        static float3 direction = (target - bulletOriginPosition).Normalized();
        if (mBulletsWave % nBullets == 0)
        {
            direction = target - bulletOriginPosition;
            direction.Normalize();
        }
        
        float3 right = mGameObject->GetUp().Cross(direction);

        for (int i : { -1, 1 })
        {
            GameObject* bulletGO = GameManager::GetInstance()->GetPoolManager()->Spawn(PoolType::BOSS_BULLET);
            float3 position = bulletOriginPosition + right * width * sin((pi*3/4)* (mBulletsWave % nBullets) / (nBullets - 1)) * i;

            Bullet* bulletScript = static_cast<Bullet*>(static_cast<ScriptComponent*>(bulletGO->GetComponent(ComponentType::SCRIPT))->GetScriptInstance());
            ColorGradient gradient;
            gradient.AddColorGradientMark(0.1f, float4(255.0f, 255.0f, 255.0f, 1.0f));
            bulletScript->Init(position, direction, mBulletSpeed, 1.0f, &gradient, mBulletsDamage, mBulletRange);


            GameManager::GetInstance()->GetAudio()->PlayOneShot(SFX::ENEMY_ROBOT_GUNFIRE, bulletGO->GetWorldPosition());
        }
        mBulletsWave++;
    }
}

void EnemyBoss::BulletHellPattern5() //Stream
{

    if (mBulletHellTimer.Delay(BEAT_TIME * 0.25)) //Each pattern will need different rythm
    {
        const unsigned int nBullets = 20;
        float alpha = (ATTACK_CONE_RADIANTS / 2) - ATTACK_CONE_RADIANTS * (mBulletsWave % nBullets) / (nBullets - 1);
        if (mBulletsWave % (2*nBullets) >= nBullets)
        {
            alpha = ATTACK_CONE_RADIANTS / (nBullets * 2) - alpha;
        }
        float3 bulletOriginPosition = mGameObject->GetWorldPosition();
        bulletOriginPosition.y = mPlayer->GetWorldPosition().y + 2.0f;

        GameObject* bulletGO = GameManager::GetInstance()->GetPoolManager()->Spawn(PoolType::BOSS_BULLET);
        float3 direction = float3(mFront.x * cos(alpha) - mFront.z * sin(alpha), mFront.y, mFront.x * sin(alpha) + mFront.z * cos(alpha));
        LookAt(direction, BEAT_TIME / 8);
        direction.Normalize();
        Bullet* bulletScript = static_cast<Bullet*>(static_cast<ScriptComponent*>(bulletGO->GetComponent(ComponentType::SCRIPT))->GetScriptInstance());
        ColorGradient gradient;
        gradient.AddColorGradientMark(0.1f, float4(255.0f, 255.0f, 255.0f, 1.0f));
        bulletScript->Init(bulletOriginPosition, direction, mBulletSpeed, 1.0f, &gradient, mBulletsDamage, mBulletRange);
        mBulletsWave++;

        // We use player shot sound because other sounds may reach the max limit defined by composer, causing some sounds to be dropped
        GameManager::GetInstance()->GetAudio()->PlayOneShot(SFX::PLAYER_PISTOL, bulletGO->GetWorldPosition()); 
    }
}

void EnemyBoss::BulletHellPattern6() //Aimed circles
{
    const float radius = 10.0f;
    if (mBulletHellTimer.Delay(BEAT_TIME * 4)) //Each pattern will need different rythm
    {
        const unsigned int nBullets = 16;
        float3 target = mPlayer->GetWorldPosition();
        LookAt(target - mGameObject->GetWorldPosition(), BEAT_TIME);
        float3 front = mPlayer->GetFront();
        for (int i = 0; i < nBullets; i++)
        {
            float alpha = 2 * pi * i / nBullets;
            float3 direction = float3(front.x * cos(alpha) - front.z * sin(alpha), front.y, front.x * sin(alpha) + front.z * cos(alpha));
            direction.Normalize();
            GameObject* bulletGO = GameManager::GetInstance()->GetPoolManager()->Spawn(PoolType::BOSS_BULLET);
            Bullet* bulletScript = static_cast<Bullet*>(static_cast<ScriptComponent*>(bulletGO->GetComponent(ComponentType::SCRIPT))->GetScriptInstance());
            ColorGradient gradient;
            gradient.AddColorGradientMark(0.1f, float4(255.0f, 255.0f, 255.0f, 1.0f));
            bulletScript->Init(target - direction*radius, direction, mBulletSpeed, 1.0f, &gradient, mBulletsDamage, mBulletRange);

            if (i % 2 == 0)
            {
                GameManager::GetInstance()->GetAudio()->PlayOneShot(SFX::ENEMY_ROBOT_GUNFIRE, bulletGO->GetWorldPosition());
            }
        }
    }
}

void EnemyBoss::UpdatePhase1()
{
    GameManager::GetInstance()->HandleBossAudio(mStage);
    static unsigned int sequence = 0;
    switch (mCurrentState)
    {
    case EnemyState::IDLE:
        if (mAttackCoolDownTimer.Delay(mAttackCoolDown))
        {
            switch (sequence)
            {
            case 0:
                StartBulletAttack(BulletPattern::CIRCLES);
                break;
            case 1:
                StartBulletAttack(BulletPattern::ARROW);
                break;
            case 2:
                StartBulletAttack(BulletPattern::WAVE);
                break;
            }
        }
        break;
    case EnemyState::CHARGING_LASER:
    case EnemyState::ATTACK:
        if (mAttackCoolDownTimer.Delay(mBulletHellDuration))
        {
            mCurrentState = EnemyState::IDLE;
            if (mAnimationComponent) mAnimationComponent->SendTrigger("tIdle", mIdleTransitionDuration);
            ++sequence;
            sequence %= 3;
        }
        break;
    }
}

void EnemyBoss::UpdatePhase3()
{
    static int test = 1;
    GameManager::GetInstance()->HandleBossAudio(mStage);
    static unsigned int sequence = -1;
    static unsigned int attack = 0;
    switch (mCurrentState)
    {
    case EnemyState::IDLE:
        if (mAttackCoolDownTimer.Delay(mAttackCoolDown))
        {
            switch (sequence)
            {
            case -1:
                BombAttack("BombingTemplate3.prfb");
                break;
            case 0:
                LaserAttack();
                break;
            case 1:
                StartBulletAttack(BulletPattern::TARGETED_CIRCLES);
                break;
            case 2:
                BombAttack("BombingTemplate2.prfb");
                break;
            case 3:
                StartBulletAttack(BulletPattern::ARROW);
                break;
            }
        }
        break;
    case EnemyState::CHARGING_LASER:
    case EnemyState::ATTACK:
        switch (sequence * 10 + attack)
        {
        case 0:
            if (mAttackCoolDownTimer.Delay(mAttackSequenceCooldown))
            {
                StartBulletAttack(BulletPattern::CIRCLES);
                attack++;
            }
            break;
        case 1:
            if (mAttackCoolDownTimer.Delay(mAttackSequenceCooldown))
            {
                BombAttack("BombingTemplate1.prfb"); // Big
                attack++;
            }
            break;
        case 10:
        case 20:
            if (mAttackCoolDownTimer.Delay(mAttackSequenceCooldown))
            {
                LaserAttack();
                attack++;
            }
            break;
        case 30:           
        case 11:
            if (mAttackCoolDownTimer.Delay(mAttackSequenceCooldown))
            {
                BombAttack("BombingTemplate3.prfb"); // Big
                attack++;
            }
            break;
        case 21:
            if (mAttackCoolDownTimer.Delay(mAttackSequenceCooldown))
            {
                StartBulletAttack(BulletPattern::WAVE);
                attack++;
            }
            break;
        case 31:
            if (mAttackCoolDownTimer.Delay(mAttackSequenceCooldown))
            {
                StartBulletAttack(BulletPattern::TARGETED_CIRCLES);
                attack++;
            }
            break;
        default:
            if (mAttackCoolDownTimer.Delay(mBulletHellDuration))
            {
                mCurrentState = EnemyState::IDLE;
                if (mAnimationComponent) mAnimationComponent->SendTrigger("tIdle", mIdleTransitionDuration);
                ++sequence;
                sequence %= 4;
                attack = 0;
            }
            break;
        }
    }
    test++;
}

void EnemyBoss::UpdatePhase2()
{
    GameManager::GetInstance()->HandleBossAudio(mStage);
    static int sequence = -1;
    static unsigned int attack = 0;
    switch (mCurrentState)
    {
    case EnemyState::IDLE:
        if (mAttackCoolDownTimer.Delay(mAttackCoolDown))
        {
            switch (sequence)
            {
            case 1:
                LaserAttack();
                break;
            case 2:
                StartBulletAttack(BulletPattern::TARGETED_CIRCLES);
                break;
            case 0:
                StartBulletAttack(BulletPattern::WAVE);
                break;
            case 3:
                StartBulletAttack(BulletPattern::CIRCLES);
                break;
            case -1:// Start with bombs. Never repeat this sequence
                LaserAttack();
                break;
            }
        }
        break;
    case EnemyState::CHARGING_LASER:
    case EnemyState::ATTACK:
        switch (sequence * 10 + attack)
        {
        case 10:
        case 31:
            if (mAttackCoolDownTimer.Delay(mAttackSequenceCooldown))
            {
                StartBulletAttack(BulletPattern::ARROW);
                attack++;
            }
            break;
        case 30:
        case 11:
            if (mAttackCoolDownTimer.Delay(mAttackSequenceCooldown))
            {
                LaserAttack();
                attack++;
            }
            break;
        case 0:
        case 20:
            if (mAttackCoolDownTimer.Delay(mAttackSequenceCooldown))
            {
                StartBulletAttack(BulletPattern::TARGETED_CIRCLES);
                attack++;
            }
            break;
        default:
            if (mAttackCoolDownTimer.Delay(mBulletHellDuration))
            {
                mCurrentState = EnemyState::IDLE;
                if (mAnimationComponent) mAnimationComponent->SendTrigger("tIdle", mIdleTransitionDuration);
                ++sequence;
                sequence %= 4;
                attack = 0;
            }
            break;
        }
    }
}

void EnemyBoss::LookAt(float3 direction, float time)
{ 
    float angle = mGameObject->GetFront().AngleBetween(direction);
    LookAt(direction.xz().Normalized(), angle / time);
}

void EnemyBoss::LookAt(float2 direction, float speed)
{
    mTargetRotation = direction.AngleBetween(float2::unitY);
    if (direction.x < 0)
    {
        mTargetRotation = 2*pi - mTargetRotation;
    }
    mRotationSpeed = speed;
}

void EnemyBoss::Rotate()
{
    if (mRotationSpeed != 0)
    {
        float deltaTime = App->GetDt();
        float angle = mTargetRotation - mGameObject->GetLocalEulerAngles().y;
        if (angle > pi) angle -= 2 * pi;

        float rotationAmount = mRotationSpeed * deltaTime;
        float3 currentRotation = mGameObject->GetLocalEulerAngles();
        if (std::abs(angle) < std::abs(rotationAmount))
        {
            mGameObject->SetLocalRotation(currentRotation + float3::unitY.Mul(angle));
            mRotationSpeed = 0.0f;
        }
        else
        {
            if (angle < 0) rotationAmount *= -1;
            mGameObject->SetLocalRotation(currentRotation + float3::unitY.Mul(rotationAmount));
        }
    }
}

void EnemyBoss::TakeDamage(float damage)
{
    if (mInvulnerable) return;
    if (mHealth > 0) // TODO: WITHOUT THIS IF DEATH is called two times
    {
        GameManager::GetInstance()->GetAudio()->PlayOneShot(SFX::BOSS_HIT);

        ActivateHitEffect();
        mHealth -= damage;

        if (mHealth <= 0 and mStage == 2)
        {
            // Use Hit2 animation before Death??
            mCurrentState = EnemyState::DEATH;
            GameManager::GetInstance()->GetAudio()->PlayOneShot(SFX::BOSS_SCREAM);
            mDeathParticlesHead->SetEnabled(true);
            mDeathParticlesBody->SetEnabled(true);
            mDeathParticlesLegs->SetEnabled(true);
        }
    }
}

void EnemyBoss::InterruptAttacks() 
{
    mBulletHell = BulletPattern::NONE;
    BossLaser* laserScript = static_cast<BossLaser*>(static_cast<ScriptComponent*>(mLaserGO->GetComponent(ComponentType::SCRIPT))->GetScriptInstance());
    if (laserScript)
    {
        laserScript->Interrupt();
    }
    for (GameObject* bombGO : mTemplates)
    {
        bombGO->SetEnabled(false);
    }
}

void EnemyBoss::SwitchMaterial(int uuid)
{
    for (size_t i = 0; i < mMeshComponents.size(); i++)
    {
        MeshRendererComponent* meshComponent = static_cast<MeshRendererComponent*>(mMeshComponents[i]);
        meshComponent->SetMaterial(uuid);
    }
}
#include <Engine/Core/Log.h>
#include <Engine/Core/Math/Math.h>
#include <Engine/Core/Math/Quaternion.h>
#include <Engine/Core/Math/Vector3.h>
#include <Engine/Level/Actor.h>
// Physics.h includes Engine/Physics/Types.h which defines RayCastHit.
#include <Engine/Physics/Physics.h>

#include "AudioTriggerComponent.h"
#include "AudioListenerComponent.h"
#include "AudioProxyComponent.h"
#include "../Core/AudioSystem.h"
#include "../Core/AudioSystemRequests.h"

// ============================================================================
//  OnEnable
// ============================================================================

void AudioTriggerComponent::OnEnable()
{
    // Resolve the sibling proxy (done by the base class).
    AudioSystemProxyDependentComponent::OnEnable();

    if (_proxy == nullptr)
        return;

    // Resolve trigger IDs from their names.
    AudioSystem* system = AudioSystem::Get();

    if (PlayTriggerName.HasChars())
        _playTriggerId = system->GetTriggerId(PlayTriggerName);
    else
        _playTriggerId = INVALID_AUDIO_SYSTEM_ID;

    if (StopTriggerName.HasChars())
        _stopTriggerId = system->GetTriggerId(StopTriggerName);
    else
        _stopTriggerId = INVALID_AUDIO_SYSTEM_ID;

    // Reset smoothing values.
    _currentOcclusion   = 0.0f;
    _targetOcclusion    = 0.0f;
    _currentObstruction = 0.0f;
    _targetObstruction  = 0.0f;

    _state = AudioSystemTriggerState::Invalid;

    if (LoadOnInit && _playTriggerId != INVALID_AUDIO_SYSTEM_ID)
        RequestLoad(false);
}

// ============================================================================
//  OnUpdate
// ============================================================================

void AudioTriggerComponent::OnUpdate()
{
    if (_state == AudioSystemTriggerState::Playing)
        UpdateObstructionOcclusion();
}

// ============================================================================
//  OnDisable
// ============================================================================

void AudioTriggerComponent::OnDisable()
{
    // Stop any active playback first (fire-and-forget, not sync).
    if (_state == AudioSystemTriggerState::Playing ||
        _state == AudioSystemTriggerState::Starting)
    {
        RequestStop(false);
    }

    // Unload if trigger data is resident.
    if (_state == AudioSystemTriggerState::Ready  ||
        _state == AudioSystemTriggerState::Stopped)
    {
        RequestUnload(false);
    }

    _state = AudioSystemTriggerState::Invalid;
    _playTriggerId = INVALID_AUDIO_SYSTEM_ID;
    _stopTriggerId = INVALID_AUDIO_SYSTEM_ID;

    AudioSystemProxyDependentComponent::OnDisable();
}

// ============================================================================
//  Play
// ============================================================================

void AudioTriggerComponent::Play(bool sync)
{
    if (_playTriggerId == INVALID_AUDIO_SYSTEM_ID)
    {
        LOG(Warning, "[AudioTriggerComponent] Play: PlayTriggerName '{0}' could not be resolved.", PlayTriggerName);
        return;
    }

    switch (_state)
    {
    case AudioSystemTriggerState::Invalid:
        // Trigger data is not loaded — load it, then play on callback.
        RequestLoad(sync);
        break;

    case AudioSystemTriggerState::Loading:
        // Already loading — PlayOnActivate flag will handle auto-play on callback.
        break;

    case AudioSystemTriggerState::Ready:
    case AudioSystemTriggerState::Stopped:
        RequestPlay(sync);
        break;

    case AudioSystemTriggerState::Starting:
    case AudioSystemTriggerState::Playing:
        // Already starting/playing — nothing to do.
        break;

    case AudioSystemTriggerState::Stopping:
        // Let the current stop complete, then the Stopped→Ready flow will handle replay.
        break;

    default:
        break;
    }
}

// ============================================================================
//  Stop
// ============================================================================

void AudioTriggerComponent::Stop(bool sync)
{
    if (_state != AudioSystemTriggerState::Playing &&
        _state != AudioSystemTriggerState::Starting)
    {
        return;
    }

    RequestStop(sync);
}

// ============================================================================
//  State queries
// ============================================================================

AudioSystemTriggerState AudioTriggerComponent::GetTriggerState() const
{
    return _state;
}

bool AudioTriggerComponent::IsLoading()  const { return _state == AudioSystemTriggerState::Loading;  }
bool AudioTriggerComponent::IsReady()    const { return _state == AudioSystemTriggerState::Ready;    }
bool AudioTriggerComponent::IsPlaying()  const { return _state == AudioSystemTriggerState::Playing;  }
bool AudioTriggerComponent::IsStopping() const { return _state == AudioSystemTriggerState::Stopping; }
bool AudioTriggerComponent::IsStopped()  const { return _state == AudioSystemTriggerState::Stopped;  }

// ============================================================================
//  RequestLoad  (private)
// ============================================================================

void AudioTriggerComponent::RequestLoad(bool sync)
{
    if (_playTriggerId == INVALID_AUDIO_SYSTEM_ID)
        return;

    if (_proxy == nullptr)
        return;

    _state = AudioSystemTriggerState::Loading;

    AudioRequest req;
    req.Type = AudioRequestType::LoadTrigger;
    req.EntityId = _proxy->GetEntityId();
    req.ObjectId = _playTriggerId;
    req.Callback = [this](bool success)
    {
        if (!success)
        {
            LOG(Warning, "[AudioTriggerComponent] LoadTrigger failed for trigger '{0}'.", PlayTriggerName);
            _state = AudioSystemTriggerState::Invalid;
            return;
        }

        _state = AudioSystemTriggerState::Ready;

        // Auto-play if the user called Play() while loading or PlayOnActivate is set.
        if (PlayOnActivate)
            RequestPlay(false);
    };

    if (sync)
        AudioSystem::Get()->SendRequestSync(std::move(req));
    else
        AudioSystem::Get()->SendRequest(std::move(req));
}

// ============================================================================
//  RequestPlay  (private)
// ============================================================================

void AudioTriggerComponent::RequestPlay(bool sync)
{
    if (_playTriggerId == INVALID_AUDIO_SYSTEM_ID)
        return;

    if (_proxy == nullptr)
        return;

    _state = AudioSystemTriggerState::Starting;

    AudioRequest req;
    req.Type = AudioRequestType::ActivateTrigger;
    req.EntityId = _proxy->GetEntityId();
    req.ObjectId = _playTriggerId;
    req.Callback = [this](bool success)
    {
        if (!success)
        {
            LOG(Warning, "[AudioTriggerComponent] ActivateTrigger failed for trigger '{0}'.", PlayTriggerName);
            _state = AudioSystemTriggerState::Ready;
            return;
        }

        _state = AudioSystemTriggerState::Playing;
    };

    if (sync)
        AudioSystem::Get()->SendRequestSync(std::move(req));
    else
        AudioSystem::Get()->SendRequest(std::move(req));
}

// ============================================================================
//  RequestStop  (private)
// ============================================================================

void AudioTriggerComponent::RequestStop(bool sync)
{
    if (_proxy == nullptr)
        return;

    _state = AudioSystemTriggerState::Stopping;

    const AudioSystemDataID entityId = _proxy->GetEntityId();

    if (_stopTriggerId != INVALID_AUDIO_SYSTEM_ID)
    {
        // Use the dedicated stop trigger.
        AudioRequest req;
        req.Type = AudioRequestType::ActivateTrigger;
        req.EntityId = entityId;
        req.ObjectId = _stopTriggerId;
        req.Callback = [this](bool success)
        {
            _state = AudioSystemTriggerState::Stopped;
        };

        if (sync)
            AudioSystem::Get()->SendRequestSync(std::move(req));
        else
            AudioSystem::Get()->SendRequest(std::move(req));
    }
    else
    {
        // No stop trigger — send a raw StopEvent.
        AudioRequest req;
        req.Type = AudioRequestType::StopEvent;
        req.EntityId = entityId;
        req.ObjectId = _playTriggerId;
        req.Callback = [this](bool success)
        {
            _state = AudioSystemTriggerState::Stopped;
        };

        if (sync)
            AudioSystem::Get()->SendRequestSync(std::move(req));
        else
            AudioSystem::Get()->SendRequest(std::move(req));
    }
}

// ============================================================================
//  RequestUnload  (private)
// ============================================================================

void AudioTriggerComponent::RequestUnload(bool sync)
{
    if (_playTriggerId == INVALID_AUDIO_SYSTEM_ID)
        return;

    if (_proxy == nullptr)
        return;

    _state = AudioSystemTriggerState::Unloading;

    AudioRequest req;
    req.Type = AudioRequestType::UnloadTrigger;
    req.EntityId = _proxy->GetEntityId();
    req.ObjectId = _playTriggerId;
    req.Callback = [this](bool success)
    {
        _state = AudioSystemTriggerState::Invalid;
    };

    if (sync)
        AudioSystem::Get()->SendRequestSync(std::move(req));
    else
        AudioSystem::Get()->SendRequest(std::move(req));
}

// ============================================================================
//  UpdateObstructionOcclusion  (private)
// ============================================================================

void AudioTriggerComponent::UpdateObstructionOcclusion()
{
    if (ObstructionType == AudioSystemSoundObstructionType::None)
        return;

    const AudioListenerComponent* listener =
        AudioSystem::Get()->GetWorldModule().GetDefaultListener();
    if (listener == nullptr)
        return;

    // Compute new target values.
    float targetObstruction = 0.0f;
    float targetOcclusion   = 0.0f;
    ComputeObstructionOcclusion(targetObstruction, targetOcclusion);

    _targetObstruction = targetObstruction;
    _targetOcclusion   = targetOcclusion;

    // Smooth toward the targets.
    _currentObstruction = Math::Lerp(_currentObstruction, _targetObstruction, k_ObstructionSmoothingFactor);
    _currentOcclusion   = Math::Lerp(_currentOcclusion,   _targetOcclusion,   k_ObstructionSmoothingFactor);

    if (_proxy == nullptr)
        return;

    AudioRequest req;
    req.Type = AudioRequestType::SetObstructionOcclusion;
    req.EntityId    = _proxy->GetEntityId();
    req.Obstruction = _currentObstruction;
    req.Occlusion   = _currentOcclusion;
    AudioSystem::Get()->SendRequest(std::move(req));
}

// ============================================================================
//  ComputeObstructionOcclusion  (private)
// ============================================================================

void AudioTriggerComponent::ComputeObstructionOcclusion(float& outObstruction, float& outOcclusion) const
{
    outObstruction = 0.0f;
    outOcclusion   = 0.0f;

    const Actor* owner = GetActor();
    if (owner == nullptr)
        return;

    const AudioListenerComponent* listener =
        AudioSystem::Get()->GetWorldModule().GetDefaultListener();
    if (listener == nullptr)
        return;

    const Actor* listenerActor = listener->GetActor();
    if (listenerActor == nullptr)
        return;

    const Vector3 sourcePos   = owner->GetPosition();
    const Vector3 listenerPos = listenerActor->GetPosition();
    const Vector3 delta       = listenerPos - sourcePos;
    const float   distance    = delta.Length();

    if (distance < ZeroTolerance)
        return;

    const Vector3 direction = delta / distance;

    if (ObstructionType == AudioSystemSoundObstructionType::SingleRay)
    {
        const bool blocked = CastRay(sourcePos, direction, distance);
        outOcclusion   = blocked ? 1.0f : 0.0f;
        outObstruction = 0.0f;
    }
    else if (ObstructionType == AudioSystemSoundObstructionType::MultipleRay)
    {
        // Build two perpendicular axes for the offset rays.
        const Vector3 right = Vector3::Cross(direction, Vector3::Up);
        const Vector3 perp  = right.IsZero() ?
            Vector3::Cross(direction, Vector3::Right) : right;

        const Vector3 perpNorm = Vector3::Normalize(perp);
        const Vector3 upNorm   = Vector3::Normalize(Vector3::Cross(direction, perpNorm));

        // Five rays: centre, +right, -right, +up, -up.
        const Vector3 offsets[k_MultiRayCount] =
        {
            Vector3::Zero,
            perpNorm * k_MultiRayOffsetRadius,
            -perpNorm * k_MultiRayOffsetRadius,
            upNorm * k_MultiRayOffsetRadius,
            -upNorm * k_MultiRayOffsetRadius,
        };

        int32 hitCount        = 0;
        bool  centreBlocked   = false;
        int32 sideHitCount    = 0;

        for (int32 i = 0; i < k_MultiRayCount; ++i)
        {
            const Vector3 origin    = sourcePos + offsets[i];
            const Vector3 rayDir    = Vector3::Normalize(listenerPos - origin);
            const float   rayDist   = Vector3::Distance(origin, listenerPos);
            const bool    blocked   = CastRay(origin, rayDir, rayDist);

            if (blocked)
            {
                ++hitCount;
                if (i == 0)
                    centreBlocked = true;
                else
                    ++sideHitCount;
            }
        }

        // Occlusion: fraction of rays that were blocked (all rays).
        outOcclusion = static_cast<float>(hitCount) / static_cast<float>(k_MultiRayCount);

        // Obstruction: occurs when side rays are blocked but the centre ray is
        // not (sound leaks around an obstacle).
        const int32 sideRayCount = k_MultiRayCount - 1;
        if (!centreBlocked && sideHitCount > 0)
            outObstruction = static_cast<float>(sideHitCount) / static_cast<float>(sideRayCount);
        else
            outObstruction = 0.0f;
    }
}

// ============================================================================
//  CastRay  (private)
// ============================================================================

bool AudioTriggerComponent::CastRay(const Vector3& start, const Vector3& direction, float distance) const
{
    RayCastHit hit;
    const uint32 layerMask = 1u << OcclusionCollisionLayer;
    return Physics::RayCast(start, direction, hit, distance, layerMask);
}

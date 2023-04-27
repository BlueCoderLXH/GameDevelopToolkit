#pragma once

#include <functional>

#include "GameEventData.h"

DEFINE_LOG_CATEGORY_STATIC(LogGameEventSystem, Log, All);

DECLARE_MULTICAST_DELEGATE_OneParam(FGameEventListener, const FGameEventData&);

typedef void (*FGameEventStaticDelegate)(const FGameEventData&);

struct FGameEventCallbackKey
{
    FGameEventCallbackKey(void* InCallbackPtr) {
        HashCode = HashCombine(PointerHash(nullptr), PointerHash(InCallbackPtr));
    }

    FGameEventCallbackKey(void* InTarget, void* InCallbackPtr) {
        HashCode = HashCombine(PointerHash(InTarget), PointerHash(InCallbackPtr));
    }

    bool operator==(const FGameEventCallbackKey& Other) const {
        return HashCode == Other.HashCode;
    }

    uint32 GetHashCode() const {
        return HashCode;
    }

private:
    uint32 HashCode;
};

inline uint32 GetTypeHash(const FGameEventCallbackKey& Key) {
    return Key.GetHashCode();
}

/**
 * FGameEventListeners
 * The collection of FGameEventListener
 */
class GAMEEVENTSYSTEM_API FGameEventListeners
{
public:
    void Init(const FGameEventType& InEventID) {
        EventID = InEventID;
    }

    bool Register(const FGameEventStaticDelegate& InCallback) {
        auto BindListener = [&] () -> FDelegateHandle {
            return GetListener().AddStatic(InCallback);
        };

        return RegisterInner(nullptr, PointerCast<void*>(InCallback), BindListener);
    }

    bool Unregister(const FGameEventStaticDelegate& InCallback) {
        auto UnBindListener = [&] (const FDelegateHandle& DelegateHandle) -> void {
            GetListener().Remove(DelegateHandle);
        };

        return UnregisterInner(nullptr, PointerCast<void*>(InCallback), UnBindListener);
    }

    template<typename ClassType>
    bool Register(
        std::enable_if_t<std::is_base_of<UObject, ClassType>::value, ClassType*> InTarget,
        void (ClassType::* InCallback) (const FGameEventData&)) {
        auto BindListener = [&] () -> FDelegateHandle {
            return GetListener(InTarget).AddUObject(InTarget, InCallback);
        };

        return RegisterInner(InTarget, PointerCast<void*>(InCallback), BindListener);
    }

    template<typename ClassType>
    bool Unregister(
        std::enable_if_t<std::is_base_of<UObject, ClassType>::value, ClassType*> InTarget,
        void (ClassType::* InCallback) (const FGameEventData&)) {
        auto UnBindListener = [&] (const FDelegateHandle& DelegateHandle) -> void {
            GetListener(InTarget).Remove(DelegateHandle);
        };

        return UnregisterInner(InTarget, PointerCast<void*>(InCallback), UnBindListener);
    }

    template<typename ClassType>
    bool RegisterRaw(ClassType* InTarget, void (ClassType::* InCallback) (const FGameEventData&)) {
        auto BindListener = [&] () -> FDelegateHandle {
            return GetListener().AddRaw(InTarget, InCallback);
        };

        return RegisterInner(InTarget, PointerCast<void*>(InCallback), BindListener);
    }

    template<typename ClassType>
    bool UnregisterRaw(ClassType* InTarget, void (ClassType::* InCallback) (const FGameEventData&)) {
        auto UnBindListener = [&] (const FDelegateHandle& DelegateHandle) -> void {
            GetListener().Remove(DelegateHandle);
        };

        return UnregisterInner(InTarget, PointerCast<void*>(InCallback), UnBindListener);
    }

    bool Dispatch(const FGameEventData& EventData);

    void Clear();

private:
    bool RegisterInner(void* InTarget, void* InCallback, std::function<FDelegateHandle()> BindListener) {
        const FGameEventCallbackKey FuncKey(InTarget, InCallback);
        if (HandleMap.Find(FuncKey)) {
            UE_LOG(LogGameEventSystem, Warning, TEXT("Register an existent callback for event:%s"), *EventID.ToString());
            return false;
        }

        const auto DelegateHandle = BindListener();
        HandleMap.Add(FuncKey, DelegateHandle);
        return true;
    }

    bool UnregisterInner(void* InTarget, void* InCallback, std::function<void(const FDelegateHandle&)> UnBindListener) {
        const FGameEventCallbackKey FuncKey(InTarget, InCallback);
        if (!HandleMap.Find(FuncKey)) {
            UE_LOG(LogGameEventSystem, Warning, TEXT("Unregister an inexistent callback for event:%s"), *EventID.ToString());
            return false;
        }

        const auto DelegateHandle = HandleMap[FuncKey];
        UnBindListener(DelegateHandle);
        HandleMap.Remove(FuncKey);
        return true;
    }

    FGameEventListener& GetListener(const UObject* WorldContext = nullptr) {
        return Listeners;
    }

    template<typename DstType, typename SrcType>
    static DstType PointerCast(SrcType SrcPtr) {
        return *static_cast<DstType*>(static_cast<void*>(&SrcPtr));
    }

    FGameEventType EventID;

    FGameEventListener Listeners;

    TMap<FGameEventCallbackKey, FDelegateHandle> HandleMap;
};

/**
 * FGameEventHandler
 * Handler the event action:
 * - Register()
 * - Unregister()
 * - Dispatch()
 */
class GAMEEVENTSYSTEM_API FGameEventHandler
{
public:
    bool Register(
        const FGameEventType& InEventID,
        const FGameEventStaticDelegate InStaticCallback);

    bool Unregister(
        const FGameEventType& InEventID,
        const FGameEventStaticDelegate InStaticCallback);

    template<typename ClassType>
    bool Register(
        const FGameEventType& InEventID,
        std::enable_if_t<std::is_base_of<UObject, ClassType>::value, ClassType*> InTarget,
        void (ClassType::* InCallback) (const FGameEventData&)) {
        
        FGameEventListeners& ListenersPtr = EventMap.FindOrAdd(InEventID);
        ListenersPtr.Init(InEventID);

        if (!ListenersPtr.Register(InTarget, InCallback)) {
            return false;
        }
        
        return true;
    }

    template<typename ClassType>
    bool Unregister(
        const FGameEventType& InEventID,
        std::enable_if_t<std::is_base_of<UObject, ClassType>::value, ClassType*> InTarget,
        void (ClassType::* InCallback) (const FGameEventData&)) {
        
        FGameEventListeners* ListenersPtr = EventMap.Find(InEventID);
        if (!ListenersPtr) {
            return false;
        }

        return ListenersPtr->Unregister(InTarget, InCallback);
    }

    template<typename ClassType>
    bool RegisterRaw(
        const FGameEventType& InEventID, ClassType* InTarget,
        void (ClassType::* InCallback) (const FGameEventData&)) {
        
        FGameEventListeners& ListenersPtr = EventMap.FindOrAdd(InEventID);
        ListenersPtr.Init(InEventID);

        if (!ListenersPtr.RegisterRaw(InTarget, InCallback)) {
            return false;
        }
        
        return true;
    }

    template<typename ClassType>
    bool UnregisterRaw(
        const FGameEventType& InEventID, ClassType* InTarget,
        void (ClassType::* InCallback) (const FGameEventData&)) {

        FGameEventListeners* ListenersPtr = EventMap.Find(InEventID);
        if (!ListenersPtr) {
            return false;
        }

        return ListenersPtr->UnregisterRaw(InTarget, InCallback);
    }

    bool Dispatch(const FGameEventData& EventData);

    void Clear();

private:
    TMap<FGameEventType, FGameEventListeners> EventMap;
};

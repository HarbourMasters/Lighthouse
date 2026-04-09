#include "EventSystem.h"
#include <algorithm>
#include <stdexcept>
#include "src/port/devtools/EventDebugger.h"

#include "port/enhancements/events/hooks/list/EngineEvent.h"

EventSystem* EventSystem::Instance = new EventSystem();

EventID EventSystem::RegisterEvent(const char* name) {
    const EventID id = this->mInternalEventID++;
    this->mEventRegistry[id] = EventRegistration{ .name = name == nullptr ? "Unknown" : name };
    return id;
}

ListenerID EventSystem::RegisterListener(EventID id, EventCallback callback, EventPriority priority, const char* file,
                                         int line) {
    if (id == -1) {
        throw std::runtime_error("Trying to register listener for unregistered event");
    }

    auto& registry = this->mEventRegistry[id];

    if (std::find_if(registry.listeners.begin(), registry.listeners.end(), [callback](const EventListener& listener) {
            return listener.function == callback;
        }) != registry.listeners.end()) {
        throw std::runtime_error("Listener already registered");
    }

    const EventListener newListener = { registry.NextListenerID++, priority, callback, { file, line, 0 } };

    auto insertIt = std::lower_bound(registry.listeners.begin(), registry.listeners.end(), newListener,
                                     [](const EventListener& existingListener, const EventListener& listenerToInsert) {
                                         return existingListener.priority < listenerToInsert.priority;
                                     });

    registry.listeners.insert(insertIt, newListener);

    return newListener.id;
}

void EventSystem::UnregisterListener(EventID id, ListenerID listenerId) {
    auto& registry = this->mEventRegistry[id];

    auto it = std::find_if(registry.listeners.begin(), registry.listeners.end(),
                           [listenerId](const EventListener& listener) { return listener.id == listenerId; });

    if (it == registry.listeners.end()) {
        return;
    }

    registry.listeners.erase(it);
}

void EventSystem::CallEvent(const EventID id, IEvent* event, const char* file, const int line, const char* key) {
    auto& registry = this->mEventRegistry[id];

    for (auto& listener : registry.listeners) {
        listener.function(event);
    }

    auto& info = registry.callers[key];

    if (info.path == nullptr) {
        info.path = file;
        info.line = line;
    }

    info.count++;
}

extern "C" EventID EventSystem_RegisterEvent(const char* name) {
    return EventSystem::Instance->RegisterEvent(name);
}

extern "C" ListenerID EventSystem_RegisterListener(EventID id, EventCallback callback, EventPriority priority,
                                                   const char* file, int line) {
    return EventSystem::Instance->RegisterListener(id, callback, priority, file, line);
}

extern "C" void EventSystem_UnregisterListener(EventID ev, ListenerID id) {
    EventSystem::Instance->UnregisterListener(ev, id);
}

extern "C" void EventSystem_CallEvent(EventID id, void* event, const char* file, int line, const char* key) {
    EventSystem::Instance->CallEvent(id, static_cast<IEvent*>(event), file, line, key);
}

extern "C" bool EventSystem_Should(VBehaviorID id, uint32_t result, ...) {
    // Only the external function can use the Variadic Function syntax
    // To pass the va args to the next caller must be done using va_list and reading the args into it
    // Because there can be N subscribers registered to each template call, the subscribers will be responsible for
    // creating a copy of this va_list to avoid incrementing the original pointer between calls
    va_list args;
    va_start(args, result);

    // Because of default argument promotion, even though our incoming "result" is just a bool, it needs to be typed as
    // an int to be permitted to be used in `va_start`, otherwise it is undefined behavior.
    // Here we downcast back to a bool for our actual hook handlers
    bool boolResult = static_cast<bool>(result);

    CALL_EVENT(VanillaBehavior, id, &boolResult, &args);

    va_end(args);
    return boolResult;
}

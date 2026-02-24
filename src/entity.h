/* Copyright (c) 2026. LetTheMiceFree. */

#ifndef ENTITY_MODULE_H
#define ENTITY_MODULE_H

#include <vector>
#include <memory>

class Entity;

// ================= Component =================
class Component
{
public:
    virtual ~Component() = default;
    virtual void Update(Entity& owner, float dt) {}
    virtual void Draw(Entity& owner, void* renderer) {}
};

// ================= Entity =================
class Entity
{
public:
    bool Alive = true;

    template<typename T, typename... Args>
    T* AddComponent(Args&&... args)
    {
        auto comp = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = comp.get();
        _components.push_back(std::move(comp));
        return ptr;
    }

    template<typename T>
    T* GetComponent()
    {
        for (auto& c : _components)
        {
            if (auto casted = dynamic_cast<T*>(c.get()))
                return casted;
        }
        return nullptr;
    }

    void Update(float dt)
    {
        for (auto& c : _components)
            c->Update(*this, dt);
    }

    void Draw(void* renderer)
    {
        for (auto& c : _components)
            c->Draw(*this, renderer);
    }

private:
    std::vector<std::unique_ptr<Component>> _components;
};

#endif
/* ENTITY_MODULE_H */

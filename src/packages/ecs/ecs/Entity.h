#pragma once

class Entity {
 private:
  int id;

 public:
  // TODO: This causes circular dependency
  class Registry* registry;

  Entity(int id) { this->id = id; };

  void Kill();

  int GetId() const;

  void Tag(const std::string& tag);

  bool HasTag(const std::string& tag) const;

  void Group(const std::string& group);

  bool BelongsToGroup(const std::string& group) const;

  template <typename TComponent, typename... TArgs>
  void AddComponent(TArgs&&... args);

  template <typename TComponent>
  void RemoveComponent();

  template <typename TComponent>
  bool HasComponent() const;

  template <typename TComponent>
  TComponent& GetComponent() const;

  Entity& operator=(const Entity& other) = default;

  bool operator==(const Entity& other) const { return id == other.id; }

  bool operator!=(const Entity& other) const { return id != other.id; }

  bool operator>(const Entity& other) const { return id > other.id; }

  bool operator<(const Entity& other) const { return id < other.id; }
};

#include <algorithm>

#include "Registry.h"

int Entity::GetId() const { return id; }

void Entity::Kill() { registry->KillEntity(*this); }

void Entity::Tag(const std::string& tag) { registry->TagEntity(*this, tag); }

bool Entity::HasTag(const std::string& tag) const {
  return registry->EntityHasTag(*this, tag);
}

void Entity::Group(const std::string& group) {
  registry->GroupEntity(*this, group);
}

bool Entity::BelongsToGroup(const std::string& group) const {
  return registry->EntityBelongsToGroup(*this, group);
}

#pragma once

struct EditableComponent {
  bool isEditable = false;

  explicit EditableComponent(const bool isEditable = true)
      : isEditable(isEditable) {}
};

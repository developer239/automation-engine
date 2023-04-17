#pragma once

#include <SDL.h>
#include <numeric>

#include "core/Renderer.h"
#include "devices/Screen.h"
#include "ecs/Registry.h"
#include "ecs/System.h"

#include "../components/BoundingBoxComponent.h"
#include "../components/DetectContoursComponent.h"
#include "../components/EditableComponent.h"

class RenderEditableComponentsGUISystem : public ECS::System {
 public:
  RenderEditableComponentsGUISystem() { RequireComponent<EditableComponent>(); }

  void Render(std::optional<Devices::Screen>& screen) {
    ImGui::Begin("Entities");
    for (auto& entity : GetSystemEntities()) {
      auto editableComponent =
          ECS::Registry::Instance().GetComponent<EditableComponent>(entity);
      if (!editableComponent.isEditable) {
        continue;
      }

      auto tag = ECS::Registry::Instance().GetEntityTag(entity);
      auto headerLabel = "Entity " + (!tag.empty() ? tag : std::to_string(entity.GetId()));
      if (ImGui::CollapsingHeader(headerLabel.c_str())) {
        //
        // Tag
        if (!tag.empty()) {
          ImGui::Text(
              "Entity tag %s",
              ECS::Registry::Instance().GetEntityTag(entity).c_str()
          );
        } else {
          ImGui::Text("Entity has no tag");
        }

        //
        // Groups
        auto groups = ECS::Registry::Instance().GetEntityGroups(entity);
        if (!groups.empty()) {
          std::string groupsLabel = std::reduce(
              groups.begin(),
              groups.end(),
              std::string(),
              [](const std::string& a, const std::string& b) {
                if (a.empty()) {
                  return b;
                }

                return a + ", " + b;
              }
          );
          ImGui::Text("Entity groups: %s", groupsLabel.c_str());
        } else {
          ImGui::Text("Entity belongs to no groups");
        }

        auto hasBoundingBox =
            ECS::Registry::Instance().HasComponent<BoundingBoxComponent>(entity
            );
        if (hasBoundingBox) {
          ImGui::Spacing();
          ImGui::Spacing();
          ImGui::Spacing();
          ImGui::Text("Bounding box");
          auto& boundingBox =
              ECS::Registry::Instance().GetComponent<BoundingBoxComponent>(
                  entity
              );

          //
          // Position
          ImGui::SliderInt(
              "bb:X",
              &boundingBox.position.x,
              0,
              *screen->width - boundingBox.size.width
          );
          ImGui::SliderInt(
              "bb:Y",
              &boundingBox.position.y,
              0,
              *screen->height - boundingBox.size.height
          );

          //
          // Size
          ImGui::SliderInt(
              "bb:Width",
              &boundingBox.size.width,
              0,
              *screen->width - boundingBox.position.x
          );
          ImGui::SliderInt(
              "bb:Height",
              &boundingBox.size.height,
              0,
              *screen->height - boundingBox.position.y
          );

          //
          // Color
          float color[3] = {
              static_cast<float>(boundingBox.color.r) / 255.0f,
              static_cast<float>(boundingBox.color.g) / 255.0f,
              static_cast<float>(boundingBox.color.b) / 255.0f};
          if (ImGui::ColorEdit3("bb:Color", color)) {
            boundingBox.color.r = static_cast<Uint8>(color[0] * 255.0f);
            boundingBox.color.g = static_cast<Uint8>(color[1] * 255.0f);
            boundingBox.color.b = static_cast<Uint8>(color[2] * 255.0f);
          }
          ImGui::SliderInt("bb:Thickness", &boundingBox.thickness, 1, 10);
        }

        //
        // Text Label
        auto hasTextLabel =
            ECS::Registry::Instance().HasComponent<TextLabelComponent>(entity);
        if (hasTextLabel) {
          ImGui::Spacing();
          ImGui::Spacing();
          ImGui::Spacing();
          ImGui::Text("Text Label");
          auto& textLabel =
              ECS::Registry::Instance().GetComponent<TextLabelComponent>(entity
              );

          //
          // Text
          char buffer[256];
          strcpy(buffer, textLabel.text.c_str());

          if (ImGui::InputText("Text", buffer, 256)) {
            textLabel.text = buffer;
          }

          //
          // Color
          float color[3] = {
              static_cast<float>(textLabel.color.r) / 255.0f,
              static_cast<float>(textLabel.color.g) / 255.0f,
              static_cast<float>(textLabel.color.b) / 255.0f};
          if (ImGui::ColorEdit3("label:Color", color)) {
            textLabel.color.r = static_cast<Uint8>(color[0] * 255.0f);
            textLabel.color.g = static_cast<Uint8>(color[1] * 255.0f);
            textLabel.color.b = static_cast<Uint8>(color[2] * 255.0f);
          }

          //
          // Position
          auto textLabelWidth = textLabel.text.length() * 10;
          auto autoLabelHeight = 40;
          auto maxPositionX = *screen->width - textLabelWidth;
          auto maxPositionY = *screen->height - autoLabelHeight;

          ImGui::SliderInt("label:X", &textLabel.position.x, 0, maxPositionX);
          ImGui::SliderInt("label:Y", &textLabel.position.y, 0, maxPositionY);
        }

        //
        // Detect Contours
        auto hasDetectContours =
            ECS::Registry::Instance().HasComponent<DetectContoursComponent>(
                entity
            );
        if (hasDetectContours) {
          ImGui::Spacing();
          ImGui::Spacing();
          ImGui::Spacing();
          ImGui::Text("Detect Contours");

          auto& detectContours =
              ECS::Registry::Instance().GetComponent<DetectContoursComponent>(entity
              );

          char buffer[256];
          strcpy(buffer, detectContours.id.c_str());

          if (ImGui::InputText("contours:Id", buffer, 256)) {
            detectContours.id = buffer;
          }

          //
          // Color
          float color[3] = {
              static_cast<float>(detectContours.bboxColor.r) / 255.0f,
              static_cast<float>(detectContours.bboxColor.g) / 255.0f,
              static_cast<float>(detectContours.bboxColor.b) / 255.0f};
          if (ImGui::ColorEdit3("contours:Color", color)) {
            detectContours.bboxColor.r = static_cast<Uint8>(color[0] * 255.0f);
            detectContours.bboxColor.g = static_cast<Uint8>(color[1] * 255.0f);
            detectContours.bboxColor.b = static_cast<Uint8>(color[2] * 255.0f);
          }

          //
          // MinSize
          ImGui::SliderInt(
              "contours:min-width",
              &detectContours.minArea.width,
              0,
              *screen->width
          );
          ImGui::SliderInt(
              "contours:min-height",
              &detectContours.minArea.height,
              0,
              *screen->height
          );

          //
          // MaxSize
          if(detectContours.maxArea.has_value()) {
            ImGui::SliderInt(
                "contours:max-width",
                &detectContours.maxArea->width,
                detectContours.minArea.width,
                *screen->width
            );
            ImGui::SliderInt(
                "contours:max-height",
                &detectContours.maxArea->height,
                detectContours.minArea.height,
                *screen->height
            );
          }
        }
      }
    }
    ImGui::End();
  }
};

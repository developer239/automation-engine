#pragma once

#include <SDL.h>

#include "core/AssetStore.h"
#include "ecs/System.h"

#include "../Components/TextLabelComponent.h"
#include "./ScreenSystem.h"

class RenderTextSystem : public ECS::System {
 public:
  RenderTextSystem() { RequireComponent<TextLabelComponent>(); }

  void Render(
      Core::Renderer& renderer, ImVec2 screenImageCursor, float screenScale
  ) {
    for (auto entity : GetSystemEntities()) {
      const auto textLabelComponent =
          ECS::Registry::Instance().GetComponent<TextLabelComponent>(entity);

      SDL_Surface* surface = TTF_RenderText_Blended(
          Core::AssetStore::Instance().GetFont(textLabelComponent.fontId).get(),
          textLabelComponent.text.c_str(),
          SDL_Color{
              static_cast<uint8_t>(textLabelComponent.color.r),
              static_cast<uint8_t>(textLabelComponent.color.g),
              static_cast<uint8_t>(textLabelComponent.color.b),
          }
      );
      texture = SDL_CreateTextureFromSurface(renderer.Get().get(), surface);
      SDL_FreeSurface(surface);

      int labelWidth = 0;
      int labelHeight = 0;

      SDL_QueryTexture(texture, nullptr, nullptr, &labelWidth, &labelHeight);
      SDL_Rect dstRect = {
          static_cast<int>(textLabelComponent.position[0]),
          static_cast<int>(textLabelComponent.position[1]),
          labelWidth,
          labelHeight};
      SDL_RenderCopy(renderer.Get().get(), texture, nullptr, &dstRect);

      ImVec2 labelPosition = ImVec2(
          screenImageCursor.x + textLabelComponent.position[0] * screenScale,
          screenImageCursor.y + textLabelComponent.position[1] * screenScale
      );
      ImGui::SetCursorScreenPos(labelPosition);
      ImGui::Image(
          (void*)(intptr_t)texture,
          ImVec2(labelWidth * screenScale, labelHeight * screenScale)
      );
    }
  }

  void Clear() { SDL_DestroyTexture(texture); }

 private:
  // TODO: support multiple textures 🤦‍♂️
  SDL_Texture* texture{};
};

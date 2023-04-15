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
          textLabelComponent.color.ToSDLColor()
      );
      SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer.Get().get(), surface);
      SDL_FreeSurface(surface);

      textures.emplace_back(texture);

      int labelWidth = 0;
      int labelHeight = 0;

      SDL_QueryTexture(texture, nullptr, nullptr, &labelWidth, &labelHeight);
      SDL_Rect dstRect = {
          textLabelComponent.position.x,
          textLabelComponent.position.y,
          labelWidth,
          labelHeight};
      SDL_RenderCopy(renderer.Get().get(), texture, nullptr, &dstRect);

      ImVec2 labelPosition = ImVec2(
          screenImageCursor.x + textLabelComponent.position.x * screenScale,
          screenImageCursor.y + textLabelComponent.position.y * screenScale
      );
      ImGui::SetCursorScreenPos(labelPosition);
      ImGui::Image(
          (void*)(intptr_t)texture,
          ImVec2(labelWidth * screenScale, labelHeight * screenScale)
      );
    }
  }

  void Clear() {
    for (auto texture : textures) {
      SDL_DestroyTexture(texture);
    }
    textures.clear();
  }

 private:
  std::vector<SDL_Texture*> textures{};
};

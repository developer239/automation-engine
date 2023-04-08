#pragma once

#include <SDL.h>

#include "ecs/System.h"
#include "core/AssetStore.h"

#include "../Components/TextLabelComponent.h"

class RenderTextSystem : public ECS::System {
  public:
    RenderTextSystem() {
      RequireComponent<TextLabelComponent>();
    }

    void Render(Core::Renderer& renderer, ImVec2 cursor) {
      for (auto entity: GetSystemEntities()) {
        const auto textLabelComponent = ECS::Registry::Instance().GetComponent<TextLabelComponent>(entity);

        SDL_Surface *surface = TTF_RenderText_Blended(
            Core::AssetStore::Instance().GetFont(textLabelComponent.fontId).get(),
            textLabelComponent.text.c_str(),
            SDL_Color {
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
            labelHeight
        };
        SDL_RenderCopy(renderer.Get().get(), texture, nullptr, &dstRect);

        ImGui::SetCursorScreenPos(cursor);
        ImGui::Image(
            (void*)(intptr_t)texture,
            ImVec2(labelWidth, labelHeight)
        );
      }
    }

    void Clear() { SDL_DestroyTexture(texture); }

   private:
    SDL_Texture* texture{};
};


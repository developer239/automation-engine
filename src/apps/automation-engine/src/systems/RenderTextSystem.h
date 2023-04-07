#pragma once

#include <SDL.h>

#include "ecs/System.h"

#include "../Components/TextLabelComponent.h"
#include "core/AssetStore.h"

class RenderTextSystem : public ECS::System {
  public:
    RenderTextSystem() {
      RequireComponent<TextLabelComponent>();
    }

    void Render(Core::Renderer& renderer, Core::AssetStore &assetStore, ECS::Registry& registry) {
      for (auto entity: GetSystemEntities()) {
        const auto textLabelComponent = registry.GetComponent<TextLabelComponent>(entity);

        SDL_Surface *surface = TTF_RenderText_Blended(
            assetStore.GetFont(textLabelComponent.fontId),
            textLabelComponent.text.c_str(),
            SDL_Color {
              // TODO: remove static casting
              static_cast<uint8_t>(textLabelComponent.color.r),
              static_cast<uint8_t>(textLabelComponent.color.g),
              static_cast<uint8_t>(textLabelComponent.color.b),
            }
        );
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer.Get().get(), surface);
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

        SDL_DestroyTexture(texture);
      }
    }
};


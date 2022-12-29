#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <map>
#include <string>

#include "./Renderer.h"

namespace Core {

class AssetStore {
 private:
  // TODO: use smart pointers
  std::map<std::string, SDL_Texture*> textures;
  std::map<std::string, TTF_Font*> fonts;

 public:
  void ClearAssets();

  void AddTexture(
      Renderer renderer, const std::string& assetId, const std::string& filePath
  );

  SDL_Texture* GetTexture(const std::string& assetId);

  void AddFont(
      const std::string& assetId, const std::string& filePath, int fontSize
  );

  // TODO: use smart pointers
  TTF_Font* GetFont(const std::string& assetId);
};

}  // namespace Core

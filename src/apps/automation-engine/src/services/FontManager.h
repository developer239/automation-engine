#pragma once

#include <SDL_ttf.h>
#include <iostream>
#include <map>
#include <memory>
#include <string>

class FontManager {
 public:
  static FontManager& Instance() {
    static FontManager instance;
    return instance;
  }

  TTF_Font& GetFont(const std::string& fontPath, int fontSize) {
    for (auto& fontTuple : fonts) {
      if (std::get<0>(fontTuple) == fontPath &&
          std::get<1>(fontTuple) == fontSize) {
        return *std::get<2>(fontTuple).get();
      }
    }

    TTF_Font* rawFont = TTF_OpenFont(fontPath.c_str(), fontSize);
    if (!rawFont) {
      std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
      throw std::runtime_error("Failed to load font");
    }

    std::unique_ptr<TTF_Font, decltype(&TTF_CloseFont)> font(
        rawFont,
        TTF_CloseFont
    );
    fonts.emplace_back(fontPath, fontSize, std::move(font));

    return *rawFont;
  }

  ~FontManager() { fonts.clear(); }

 private:
  FontManager() = default;
  FontManager(const FontManager&) = delete;
  FontManager& operator=(const FontManager&) = delete;

  std::vector<std::tuple<
      std::string, int, std::unique_ptr<TTF_Font, decltype(&TTF_CloseFont)>>>
      fonts;
};
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>
#include <filesystem>
#include <vector>
#include <functional>

std::string GetRootPath() {
    std::filesystem::path fullPath = std::filesystem::current_path();
    std::string rootPath = fullPath.string();

    std::string duplicate = "\\" + fullPath.filename().string();
    if (rootPath.size() >= duplicate.size() &&
        rootPath.compare(rootPath.size() - duplicate.size(), duplicate.size(), duplicate) == 0) {
        rootPath.erase(rootPath.size() - duplicate.size());
    }

    return rootPath;
}

void loadButtonTextures(const std::string& rootPath, const std::vector<std::string>& buttonPaths, std::vector<sf::Texture>& buttonTextures) {
    for (size_t i = 0; i < buttonPaths.size(); ++i) {
        if (!buttonTextures[i].loadFromFile(rootPath + buttonPaths[i])) {
            std::cerr << "Failed to load button texture: " << i << std::endl;
            exit(1);
        }
    }
}

void setPositionForButtons(const sf::Vector2u& windowSize, std::vector<sf::Sprite>& buttons, float buttonWidth, float buttonSpacing, float buttonMarginBottom) {
    float totalButtonsWidth = (buttonWidth + buttonSpacing) * buttons.size() - buttonSpacing;
    float blockStartX = (windowSize.x - totalButtonsWidth) / 2;

    for (size_t i = 0; i < buttons.size(); ++i) {
        buttons[i].setPosition(blockStartX + i * (buttonWidth + buttonSpacing), windowSize.y - buttonMarginBottom - buttonWidth);
    }
}

void handleButtonPress(sf::Sprite& button, float targetAlpha) {
    sf::Color originalColor = button.getColor();
    button.setColor(sf::Color(originalColor.r, originalColor.g, originalColor.b, static_cast<sf::Uint8>(targetAlpha)));
}

void loadAudioFiles(const std::string& folderPath, std::vector<std::string>& audioFiles) {
    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        std::string filePath = entry.path().string();
        if (filePath.substr(filePath.find_last_of(".") + 1) == "mp3") {
            audioFiles.push_back(filePath);
        }
    }
}

void handlePlayButtonPress(sf::Music& music, const std::vector<std::string>& audioFiles, int& currentTrackIndex, sf::Sprite& button, std::vector<sf::Sprite>& buttons, sf::Clock& fadeTimer, sf::Sprite*& activeButton) {
    if (!audioFiles.empty()) {
        music.openFromFile(audioFiles[currentTrackIndex]);
        music.play();
        if (activeButton != &button) {
            handleButtonPress(button, 0);
            for (size_t j = 1; j < buttons.size(); ++j)
                handleButtonPress(buttons[j], 0.5 * 255);
            activeButton = &button;
            fadeTimer.restart();
        }
    }
}

void handleStopButtonPress(sf::Music& music, sf::Sprite& button, std::vector<sf::Sprite>& buttons, sf::Clock& fadeTimer, sf::Sprite*& activeButton) {
    music.stop();
    if (activeButton != &button) {
        handleButtonPress(button, 0);
        for (size_t j = 0; j < buttons.size(); ++j) {
            if (&buttons[j] != &button)
                handleButtonPress(buttons[j], 0.5 * 255);
        }
        activeButton = &button;
        fadeTimer.restart();
    }
}

void handleNextButtonPress(sf::Music& music, const std::vector<std::string>& audioFiles, int& currentTrackIndex, sf::Sprite& button, std::vector<sf::Sprite>& buttons, sf::Clock& fadeTimer, sf::Sprite*& activeButton) {
    if (!audioFiles.empty()) {
        music.stop();
        currentTrackIndex = (currentTrackIndex + 1) % audioFiles.size();
        music.openFromFile(audioFiles[currentTrackIndex]);
        music.play();
        if (activeButton != &button) {
            handleButtonPress(button, 0);
            for (size_t j = 0; j < buttons.size(); ++j) {
                if (&buttons[j] != &button)
                    handleButtonPress(buttons[j], 0.5 * 255);
            }
            activeButton = &button;
            fadeTimer.restart();
        }
    }
}

void handlePreviousButtonPress(sf::Music& music, const std::vector<std::string>& audioFiles, int& currentTrackIndex, sf::Sprite& button, std::vector<sf::Sprite>& buttons, sf::Clock& fadeTimer, sf::Sprite*& activeButton) {
    if (!audioFiles.empty()) {
        music.stop();
        currentTrackIndex = (currentTrackIndex - 1 + audioFiles.size()) % audioFiles.size();
        music.openFromFile(audioFiles[currentTrackIndex]);
        music.play();
        if (activeButton != &button) {
            handleButtonPress(button, 0);
            for (size_t j = 0; j < buttons.size(); ++j) {
                if (&buttons[j] != &button)
                    handleButtonPress(buttons[j], 0.5 * 255);
            }
            activeButton = &button;
            fadeTimer.restart();
        }
    }
}

void processEvents(sf::RenderWindow& window, std::vector<sf::Sprite>& buttons, sf::Music& music, std::vector<std::string>& audioFiles, int& currentTrackIndex, sf::Clock& fadeTimer, sf::Sprite*& activeButton, sf::RectangleShape& volumeSlider, sf::CircleShape& volumeIndicator, bool& isVolumeIndicatorDragged) {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();

        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                for (size_t i = 0; i < buttons.size(); ++i) {
                    if (buttons[i].getGlobalBounds().contains(sf::Vector2f(event.mouseButton.x, event.mouseButton.y))) {
                        switch (i) {
                        case 0: // Play button
                            handlePlayButtonPress(music, audioFiles, currentTrackIndex, buttons[0], buttons, fadeTimer, activeButton);
                            break;
                        case 1: // Stop button
                            handleStopButtonPress(music, buttons[1], buttons, fadeTimer, activeButton);
                            break;
                        case 2: // Next button
                            handleNextButtonPress(music, audioFiles, currentTrackIndex, buttons[2], buttons, fadeTimer, activeButton);
                            break;
                        case 3: // Previous button
                            handlePreviousButtonPress(music, audioFiles, currentTrackIndex, buttons[3], buttons, fadeTimer, activeButton);
                            break;
                        }
                    }
                }

                if (volumeIndicator.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    isVolumeIndicatorDragged = true;
                }
            }
        }
        else if (event.type == sf::Event::MouseButtonReleased) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                isVolumeIndicatorDragged = false;
            }
        }
        else if (event.type == sf::Event::MouseMoved) {
            if (isVolumeIndicatorDragged) {
                float newX = event.mouseMove.x - volumeSlider.getPosition().x;
                newX = std::max(0.f, std::min(newX, volumeSlider.getSize().x));
                float volumePercentage = newX / volumeSlider.getSize().x;
                volumeIndicator.setPosition(volumeSlider.getPosition().x + newX, volumeIndicator.getPosition().y);
                music.setVolume(volumePercentage * 100);
            }
        }
    }
}

void draw(sf::RenderWindow& window, const std::vector<sf::Sprite>& buttons, const sf::Text& trackNameText, const sf::RectangleShape& volumeSlider, const sf::CircleShape& volumeIndicator, sf::Sprite& backgroundImage) {
    window.clear(sf::Color::White);
    window.draw(backgroundImage);
    for (const auto& button : buttons)
        window.draw(button);
    window.draw(trackNameText);
    window.draw(volumeSlider);
    window.draw(volumeIndicator);
    window.display();
}

int main(int argc, char* argv[]) {
    std::string rootPath = GetRootPath();
    std::string folderPath = "C:\\Users\\User\\Music";
    std::vector<std::string> audioFiles;

    loadAudioFiles(folderPath, audioFiles);

    sf::RenderWindow window(sf::VideoMode(600, 800), "Audio Player");

    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile(rootPath + "\\Assets\\background.jpg")) {
        std::cerr << "Failed to load background image" << std::endl;
        return 1;
    }

    sf::Sprite backgroundImage(backgroundTexture);

    std::vector<sf::Texture> buttonTextures(4);
    std::vector<sf::Sprite> buttons(4);
    std::vector<std::string> buttonPaths = { "\\Assets\\play-button.png", "\\Assets\\pause-button.png","\\Assets\\previous-button.png", "\\Assets\\next-button.png" };

    loadButtonTextures(rootPath, buttonPaths, buttonTextures);

    float buttonWidth = 64;
    float buttonSpacing = 54;
    float buttonMarginBottom = 100;

    for (size_t i = 0; i < buttons.size(); ++i) {
        buttons[i].setTexture(buttonTextures[i]);
    }

    setPositionForButtons(window.getSize(), buttons, buttonWidth, buttonSpacing, buttonMarginBottom);

    sf::Music music;
    int currentTrackIndex = 0;

    sf::Clock buttonTimer;
    sf::Clock fadeTimer;
    float fadeDuration = 0.25f;
    sf::Sprite* activeButton = nullptr;

    sf::RectangleShape volumeSlider(sf::Vector2f(buttons.size() * (buttonWidth + buttonSpacing) - buttonSpacing, 5));
    volumeSlider.setFillColor(sf::Color::Black);
    volumeSlider.setPosition(buttons[0].getPosition().x, buttons[0].getPosition().y - 30);

    sf::CircleShape volumeIndicator(10);
    volumeIndicator.setFillColor(sf::Color::Black);
    volumeIndicator.setOrigin(10, 10);
    volumeIndicator.setPosition(buttons[0].getPosition().x + volumeSlider.getSize().x, buttons[0].getPosition().y - 30);

    bool isVolumeIndicatorDragged = false;

    sf::Font font;
    if (!font.loadFromFile(rootPath + "\\Assets\\sf-pro-text-11.ttf")) {
        std::cerr << "Failed to load font" << std::endl;
        return 1;
    }

    sf::Text trackNameText("", font, 20);
    trackNameText.setFillColor(sf::Color::Black);
    trackNameText.setStyle(sf::Text::Bold);
    trackNameText.setPosition(buttons[0].getPosition().x, buttons[0].getPosition().y - 30 - 30);

    while (window.isOpen()) {
        processEvents(window, buttons, music, audioFiles, currentTrackIndex, fadeTimer, activeButton, volumeSlider, volumeIndicator, isVolumeIndicatorDragged);

        if (fadeTimer.getElapsedTime().asSeconds() < fadeDuration) {
            float t = fadeTimer.getElapsedTime().asSeconds() / fadeDuration;
            float alpha = t;
            if (activeButton)
                handleButtonPress(*activeButton, alpha * 255);
        }

        if (!audioFiles.empty()) {
            std::string trackName = std::filesystem::path(audioFiles[currentTrackIndex]).filename().string();
            trackNameText.setString(trackName);

            float textWidth = trackNameText.getLocalBounds().width;
            float centerX = (window.getSize().x) / 2;

            static float textOffset = 0;
            textOffset += 0.03f;
            if (textOffset > textWidth + 500)
                textOffset = -500;
            trackNameText.setPosition(centerX - textOffset, buttons[0].getPosition().y - 100);
        }

        draw(window, buttons, trackNameText, volumeSlider, volumeIndicator, backgroundImage);
    }

    return 0;
}

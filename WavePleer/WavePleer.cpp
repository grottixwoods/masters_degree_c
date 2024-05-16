#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>
#include <filesystem>
#include <vector>
#include <functional>
#include <unordered_set>
#include <fstream>

std::string GetRootPath() {
    // Получаем полный путь текущей рабочей директории
    std::filesystem::path fullPath = std::filesystem::current_path();

    // Преобразуем полный путь в строку
    std::string rootPath = fullPath.string();

    // Формируем строку для поиска дубликата
    std::string duplicate = "\\" + fullPath.filename().string();

    // Проверяем, является ли конец rootPath дубликатом 'duplicate'
    if (rootPath.size() >= duplicate.size() &&
        rootPath.compare(rootPath.size() - duplicate.size(), duplicate.size(), duplicate) == 0) {
        rootPath.erase(rootPath.size() - duplicate.size());
    }

    return rootPath;
}

void loadButtonTextures(const std::string& rootPath, const std::vector<std::string>& buttonPaths, std::vector<sf::Texture>& buttonTextures) {
    for (size_t i = 0; i < buttonPaths.size(); ++i) {

        // Проверяем, загружена ли текстура для данной кнопки из файла.
        if (!buttonTextures[i].loadFromFile(rootPath + buttonPaths[i])) {
            std::cerr << "Failed to load button texture: " << i << std::endl;
            exit(1);
        }
    }
}

void setPositionForButtons(const sf::Vector2u& windowSize, std::vector<sf::Sprite>& buttons, float buttonWidth, float buttonSpacing, float buttonMarginBottom) {
    // Вычисляем общую ширину блока кнопок с учетом промежутков между кнопками.
    float totalButtonsWidth = (buttonWidth + buttonSpacing) * buttons.size() - buttonSpacing;

    // Вычисляем начальную позицию блока кнопок по оси X, чтобы они располагались по центру окна.
    float blockStartX = (windowSize.x - totalButtonsWidth) / 2;

    // Устанавливаем позиции для каждой кнопки внутри блока.
    for (size_t i = 0; i < buttons.size(); ++i) {
        buttons[i].setPosition(blockStartX + i * (buttonWidth + buttonSpacing), windowSize.y - buttonMarginBottom - buttonWidth);
    }
}

void handleButtonPress(sf::Sprite& button, float targetAlpha) {
    // Получаем оригинальный цвет кнопки.
    sf::Color originalColor = button.getColor();

    // Устанавливаем новый цвет кнопки с тем же значением RGB, но изменяем альфа-канал (прозрачность).
    button.setColor(sf::Color(originalColor.r, originalColor.g, originalColor.b, static_cast<sf::Uint8>(targetAlpha)));
}

void loadAudioFiles(const std::string& folderPath, std::vector<std::string>& audioFiles) {
    // Перебираем все файлы и поддиректории в указанной директории (folderPath).
    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        // Получаем путь к текущему файлу или поддиректории.
        std::string filePath = entry.path().string();

        // Проверяем, что файл имеет расширение ".mp3".
        if (filePath.substr(filePath.find_last_of(".") + 1) == "mp3") {

            // Добавляем его путь в вектор audioFiles.
            audioFiles.push_back(filePath);
        }
    }
}

void handlePlayButtonPress(sf::Music& music, const std::vector<std::string>& audioFiles, int& currentTrackIndex, sf::Sprite& button, std::vector<sf::Sprite>& buttons, sf::Clock& fadeTimer, sf::Sprite*& activeButton) {
    // Проверяем, что вектор audioFiles не пустой.
    if (!audioFiles.empty()) {
        // Открываем и воспроизводим выбранный аудиофайл.
        music.openFromFile(audioFiles[currentTrackIndex]);
        music.play();

        // Проверяем активность кнопки (activeButton)
        if (activeButton != &button) {

            // Устанавливаем прозрачность для всех кнопок (кроме текущей) в прозрачное состояние.
            handleButtonPress(button, 0);
            for (size_t j = 1; j < buttons.size(); ++j)
                handleButtonPress(buttons[j], 0.5 * 255);
            activeButton = &button;

            // Запускаем таймер затухания.
            fadeTimer.restart();
        }
    }
}

void handleStopButtonPress(sf::Music& music, sf::Sprite& button, std::vector<sf::Sprite>& buttons, sf::Clock& fadeTimer, sf::Sprite*& activeButton) {
    // Останавливаем воспроизведение музыки.
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
        // Останавливаем воспроизведение музыки.
        music.stop();

        // Переключиться на следующий трек в списке плейлиста
        currentTrackIndex = (currentTrackIndex + 1) % audioFiles.size();

        // Открыть новый трек для воспроизведения.
        music.openFromFile(audioFiles[currentTrackIndex]);

        // Воспроизвести новый трек.
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
        // Останавливаем воспроизведение музыки.
        music.stop();

        // Переключиться на предыдущий трек в списке плейлиста
        currentTrackIndex = (currentTrackIndex - 1 + audioFiles.size()) % audioFiles.size();

        // Открыть новый трек для воспроизведения.
        music.openFromFile(audioFiles[currentTrackIndex]);

        // Воспроизвести новый трек.
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

void saveFavoritesToFile(const std::string& filePath, const std::unordered_set<std::string>& favorites) {
    // Открываем файл для записи.
    std::ofstream file(filePath);

    // Проверяем, удалось ли открыть файл.
    if (file.is_open()) {

        // Перебираем все элементы множества favorites и записываем их в файл.
        for (const auto& track : favorites) {

            // Записываем элемент и добавляем символ новой строки.
            file << track << std::endl;
        }
        file.close();
    }
    else {
        std::cerr << "Failed to open favorites file for writing: " << filePath << std::endl;
    }
}

void loadFavoritesFromFile(const std::string& filePath, std::unordered_set<std::string>& favorites) {
    // Открываем файл для чтения.
    std::ifstream file(filePath);

    // Проверяем, удалось ли открыть файл.
    if (file.is_open()) {
        std::string line;

        // Считываем файл построчно и добавляем каждую строку в множество favorites.
        while (std::getline(file, line)) {
            favorites.insert(line);
        }
        file.close();
    }
    else {
        std::cerr << "Failed to open favorites file for reading: " << filePath << std::endl;
    }
}

void handleFavoriteButtonPress(const std::string& currentTrack, std::unordered_set<std::string>& favorites, const std::string& favoritesFilePath) {
    // Проверяем, не содержится ли текущий трек уже в избранном.
    if (favorites.find(currentTrack) == favorites.end()) {

        // Если трек не содержится в избранном, добавляем его.
        favorites.insert(currentTrack);

        // Сохраняем обновленный список избранных в файл.
        saveFavoritesToFile(favoritesFilePath, favorites);
        std::cout << "Added to favorites: " << currentTrack << std::endl;
    }
    else {
        std::cout << "Track is already in favorites: " << currentTrack << std::endl;
    }
}

void displayFavoritesScreen(sf::RenderWindow& window, const std::unordered_set<std::string>& favorites, sf::Font& font) {
    // Создаем текстовый объект для отображения списка избранных треков.
    sf::Text favoritesText;

    // Настройки объекта текста
    favoritesText.setFont(font);
    favoritesText.setFillColor(sf::Color::Black);
    favoritesText.setCharacterSize(24);
    favoritesText.setStyle(sf::Text::Bold);
    favoritesText.setPosition(50, 50);

    // Формируем строку для списка избранных треков.
    std::string favoritesList = "Favorites:\n";
    for (const auto& track : favorites) {

        // Извлекаем имя файла из полного пути к треку и добавляем его к списку.
        favoritesList += std::filesystem::path(track).filename().string() + "\n";
    }

    // Устанавливаем сформированную строку в текстовый объект.
    favoritesText.setString(favoritesList);

    // Основной цикл отображения экрана избранных треков.
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                return;
            }
        }

        window.clear(sf::Color::White);
        window.draw(favoritesText);
        window.display();
    }
}


void loadImages(const std::string& rootPath, std::vector<sf::Texture>& images) {
    // Формируем путь к каталогу с обложками (covers).
    std::string coversPath = rootPath + "\\Covers";

    // Переменная для индекса изображения.
    int index = 0;

    // Итерируем по содержимому каталога с обложками.
    for (const auto& entry : std::filesystem::directory_iterator(coversPath)) {
        std::string filePath = entry.path().string();
        if (filePath.substr(filePath.find_last_of(".") + 1) == "png") {
            // Создаем текстуру для загрузки изображения.
            sf::Texture texture;

            // Загружаем изображение (текстуру) из файла.
            if (texture.loadFromFile(filePath)) {
                images.push_back(texture);
                index++;
            }
        }
    }
}

void setPositionForImage(sf::RenderWindow& window, sf::Sprite& imageSprite, const sf::RectangleShape& volumeSlider) {
    // Получаем размеры окна
    sf::Vector2u windowSize = window.getSize();

    // Получаем глобальные границы спрайта изображения
    sf::FloatRect imageBounds = imageSprite.getGlobalBounds();

    // Размер изображения
    sf::Vector2f desiredSize(420.f, 420.f);

    // Текущий размер спрайта изображения
    sf::Vector2f currentSize = sf::Vector2f(imageBounds.width, imageBounds.height);

    // Масштабируем спрайт
    if (currentSize != desiredSize) {
        float scaleX = desiredSize.x / currentSize.x;
        float scaleY = desiredSize.y / currentSize.y;
        imageSprite.setScale(scaleX, scaleY);
    }

    // Получаем обновленные границы спрайта
    imageBounds = imageSprite.getGlobalBounds();

    // Вычисляем позицию спрайта изображения
    imageSprite.setPosition((windowSize.x - imageBounds.width) / 2,
        volumeSlider.getPosition().y - 120 - imageBounds.height);
}

void processEvents(sf::RenderWindow& window, std::vector<sf::Sprite>& buttons, sf::Music& music, std::vector<std::string>& audioFiles, int& currentTrackIndex, sf::Clock& fadeTimer, sf::Sprite*& activeButton, sf::RectangleShape& volumeSlider, sf::CircleShape& volumeIndicator, bool& isVolumeIndicatorDragged, std::vector<sf::Texture>& images, int& currentImageIndex, sf::Sprite& imageSprite, std::unordered_set<std::string>& favorites, const std::string& favoritesFilePath, sf::Font& font) {
    sf::Event event;

    // Обрабатываем все события в очереди
    while (window.pollEvent(event)) {
        // Обработка события закрытия окна
        if (event.type == sf::Event::Closed)
            window.close();

        // Обработка события нажатия кнопки мыши
        if (event.type == sf::Event::MouseButtonPressed) {

            // Проверяем нажатие на кнопки
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
                            currentImageIndex = (currentImageIndex + 1) % images.size();
                            imageSprite.setTexture(images[currentImageIndex]);
                            setPositionForImage(window, imageSprite, volumeSlider);
                            break;
                        case 3: // Previous button
                            handlePreviousButtonPress(music, audioFiles, currentTrackIndex, buttons[3], buttons, fadeTimer, activeButton);
                            currentImageIndex = (currentImageIndex - 1 + images.size()) % images.size();
                            imageSprite.setTexture(images[currentImageIndex]);
                            setPositionForImage(window, imageSprite, volumeSlider);
                            break;
                        case 4: // Favorite button
                            if (!audioFiles.empty()) {
                                handleFavoriteButtonPress(audioFiles[currentTrackIndex], favorites, favoritesFilePath);
                            }
                            break;
                        }
                    }
                }

                // Проверяем нажатие на индикатор громкости
                if (volumeIndicator.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    isVolumeIndicatorDragged = true;
                }
            }
        }

        // Обработка события отпускания кнопки мыши
        else if (event.type == sf::Event::MouseButtonReleased) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                isVolumeIndicatorDragged = false;
            }
        }

        // Обработка события перемещения мыши
        else if (event.type == sf::Event::MouseMoved) {
            if (isVolumeIndicatorDragged) {
                float newX = event.mouseMove.x - volumeSlider.getPosition().x;
                newX = std::max(0.f, std::min(newX, volumeSlider.getSize().x));
                float volumePercentage = newX / volumeSlider.getSize().x;
                volumeIndicator.setPosition(volumeSlider.getPosition().x + newX, volumeIndicator.getPosition().y);
                music.setVolume(volumePercentage * 100);
            }
        }

        // Обработка события нажатия клавиши F для отображения списка избранного
        else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F) {
            displayFavoritesScreen(window, favorites, font);
        }
    }
}

void draw(sf::RenderWindow& window, const std::vector<sf::Sprite>& buttons, const sf::Text& trackNameText, const sf::RectangleShape& volumeSlider, const sf::CircleShape& volumeIndicator, const sf::Sprite& imageSprite) {
    // Очищаем окно, заполняя его белым цветом
    window.clear(sf::Color::White);
    
    // Отрисовываем обьекты
    for (const auto& button : buttons)
        window.draw(button);
    window.draw(trackNameText);
    window.draw(volumeSlider);
    window.draw(volumeIndicator);
    window.draw(imageSprite);
    window.display();
}

int main(int argc, char* argv[]) {
    std::string rootPath = GetRootPath();
    std::string folderPath = "C:\\Users\\Grotti\\Music";

    // Вектор для хранения путей к аудиофайлам
    std::vector<std::string> audioFiles;

    // Множество для хранения избранных аудиофайлов
    std::unordered_set<std::string> favorites;

    // Загружаем список аудиофайлов из указанной папки
    loadAudioFiles(folderPath, audioFiles);

    // Создаем графическое окно для отображения интерфейса
    sf::RenderWindow window(sf::VideoMode(600, 800), "Audio Player");

    // Загружаем текстуры для кнопок управления
    std::vector<sf::Texture> buttonTextures(5);
    std::vector<sf::Sprite> buttons(5);
    std::vector<std::string> buttonPaths = { "\\Assets\\play-button.png", "\\Assets\\pause-button.png", "\\Assets\\previous-button.png", "\\Assets\\next-button.png", "\\Assets\\favorite-button.png" };
    loadButtonTextures(rootPath, buttonPaths, buttonTextures);

    // Устанавливаем позиции для кнопок на экране
    float buttonWidth = 64;
    float buttonSpacing = 54;
    float buttonMarginBottom = 100;

    // Устанавливаем текстуры для кнопок
    for (size_t i = 0; i < buttons.size(); ++i) {
        buttons[i].setTexture(buttonTextures[i]);
    }

    setPositionForButtons(window.getSize(), buttons, buttonWidth, buttonSpacing, buttonMarginBottom);

    // Инициализируем объект для воспроизведения музыки
    sf::Music music;
    int currentTrackIndex = 0;

    // Таймер для эффекта затухания кнопок
    sf::Clock buttonTimer;
    sf::Clock fadeTimer;
    float fadeDuration = 0.25f;
    sf::Sprite* activeButton = nullptr;

    // Создаем ползунок громкости
    sf::RectangleShape volumeSlider(sf::Vector2f(buttons.size() * (buttonWidth + buttonSpacing) - buttonSpacing, 5));
    volumeSlider.setFillColor(sf::Color::Black);
    volumeSlider.setPosition(buttons[0].getPosition().x, buttons[0].getPosition().y - 30);

    // Создаем индикатор громкости
    sf::CircleShape volumeIndicator(10);
    volumeIndicator.setFillColor(sf::Color::Black);
    volumeIndicator.setOrigin(10, 10);
    volumeIndicator.setPosition(buttons[0].getPosition().x + volumeSlider.getSize().x, buttons[0].getPosition().y - 30);

    // Переменная перетаскивания индикатора громкости
    bool isVolumeIndicatorDragged = false;

    // Загружаем изображения для отображения
    std::vector<sf::Texture> images;
    loadImages(rootPath, images);

    // Индекс изображения
    int currentImageIndex = 0;

    sf::Sprite imageSprite;

    // Устанавливаем изображение, если оно доступно
    if (!images.empty()) {
        imageSprite.setTexture(images[currentImageIndex]);
        setPositionForImage(window, imageSprite, volumeSlider);
    }

    // Загружаем шрифт для отображения текста
    sf::Font font;
    if (!font.loadFromFile(rootPath + "\\Assets\\sf-pro-text-11.ttf")) {
        std::cerr << "Failed to load font" << std::endl;
        return 1;
    }

    // Текст для отображения имени текущего трека
    sf::Text trackNameText("", font, 20);
    trackNameText.setFillColor(sf::Color::Black);
    trackNameText.setStyle(sf::Text::Bold);
    trackNameText.setPosition(buttons[0].getPosition().x, buttons[0].getPosition().y - 30 - 30);

    // Загружаем избранные треки из файла
    std::string favoritesFilePath = rootPath + "\\favorites.txt";
    loadFavoritesFromFile(favoritesFilePath, favorites);

    // Основной цикл обработки событий
    while (window.isOpen()) {
        processEvents(window, buttons, music, audioFiles, currentTrackIndex, fadeTimer, activeButton, volumeSlider, volumeIndicator, isVolumeIndicatorDragged, images, currentImageIndex, imageSprite, favorites, favoritesFilePath, font);
        
        // Применение эффекта затухания кнопок
        if (fadeTimer.getElapsedTime().asSeconds() < fadeDuration) {
            float t = fadeTimer.getElapsedTime().asSeconds() / fadeDuration;
            float alpha = t;
            if (activeButton)
                handleButtonPress(*activeButton, alpha * 255);
        }
        // Отображение имени текущего трека с анимацией
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

        // Отрисовка элементов на экране
        draw(window, buttons, trackNameText, volumeSlider, volumeIndicator, imageSprite);
    }

    return 0;
}

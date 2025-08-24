/**
 * GameResourceManager.cpp
 * Implementation file for the game resource manager.
 * Responsible for constructing resource paths, loading level maps, 
 * and managing the image resource cache.
 * (Version: uses lv_draw_buf_dup and lv_draw_buf_destroy)
 */

#include "GameResourceManager.h"
#include <cstdio>
#include <time.h>
#include "Brick/Brick.h"
#include "breakout.h"
int GameResourceManager::currentLevel = 1;
// Define the static member variable for image caching, now storing lv_draw_buf_t*
std::map<std::string, lv_draw_buf_t*> GameResourceManager::s_imageCache;

GameResourceManager::GameResourceManager() : m_audioCtl(nullptr) {}
GameResourceManager::~GameResourceManager() {
    stopAudio();
}

/**
 * @brief Frees all cached image resources.
 * Equivalent to the `cleanup_resources` function from your example.
 */
void GameResourceManager::cleanupCache() {
    printf("[ResourceManager] Cleaning up image cache, %zu items to delete.\n", s_imageCache.size());
    for (auto const& [key, buf] : s_imageCache) {
        if (buf != nullptr) {
            lv_draw_buf_destroy(buf); // Use LVGL function to destroy draw_buf
        }
    }
    s_imageCache.clear();
    printf("[ResourceManager] Cache cleanup complete.\n");
}


/**
 * @brief Get the current game level, ensuring it stays within the valid range (1–6).
 * If currentLevel is less than 1 or greater than 6, it resets to 1.
 * @return The current level number.
 */
int GameResourceManager::getCurrentLevel() {
    if (currentLevel < 1) {
        currentLevel = 1;
    } else if (currentLevel > 6) {
        currentLevel = 1;
    } 
    return currentLevel;
}

/**
 * @brief Set the current game level.
 * @param level The new level number to set.
 * Note: The level is not automatically clamped here; getCurrentLevel() enforces valid range.
 */
void GameResourceManager::setCurrentLevel(int level) {
    currentLevel = level;
}

/**
 * @brief Retrieves an icon resource. Loads and caches it if not already cached.
 * Core logic corresponding to your `app_create_main_page` example.
 */
const lv_draw_buf_t* GameResourceManager::getIconSource(const std::string& iconName) {
    // 1. Check cache
    auto it = s_imageCache.find(iconName);
    if (it != s_imageCache.end()) {
        return it->second;
    }

    // 2. Load and decode
    std::string fullPath = getIconsPath() + "/" + iconName;

    lv_image_decoder_dsc_t decoder_dsc;
    lv_result_t res = lv_image_decoder_open(&decoder_dsc, fullPath.c_str(), NULL);

    if (res != LV_RESULT_OK) {
        printf("[ERROR] Failed to open image decoder: %s\n", fullPath.c_str());
        return nullptr;
    }

    // 3. Copy the decoded image data to a permanent draw_buf
    lv_draw_buf_t* permanent_buf = lv_draw_buf_dup(decoder_dsc.decoded);

    // 4. Close decoder and free temporary resources
    lv_image_decoder_close(&decoder_dsc);

    if (permanent_buf == nullptr) {
        printf("[ERROR] Failed to duplicate draw buffer.\n");
        return nullptr;
    }

    // 5. Cache and return
    s_imageCache[iconName] = permanent_buf;
    //printf("[ResourceManager] New image cached: %s\n", iconName.c_str());

    return permanent_buf;
}

// --- Resource path accessors ---

std::string GameResourceManager::getResPath() {
    return std::string(RES_ROOT) + "/res";
}

std::string GameResourceManager::getMapPath() {
    return getResPath() + "/map";
}

std::string GameResourceManager::getFontsPath() {
    return getResPath() + "/fonts";
}

std::string GameResourceManager::getIconsPath() {
    return getResPath() + "/icons";
}

std::string GameResourceManager::getAudioPath() {
    return getResPath() + "/audio";
}

/**
 * @brief Loads a map from file and populates the game brick list.
 */
bool GameResourceManager::loadMap(const std::string& mapName, Game* game) {
    std::string fullPath = getMapPath() + "/" + mapName;
    const int BRICK_COLS = 80;
    const int BRICK_ROWS = 35;
    const float BRICK_WIDTH = (float)GAME_AREA_WIDTH / BRICK_COLS;
    FILE* file = fopen(fullPath.c_str(), "r");
    if (!file) {
        return false;
    }
    char line[BRICK_COLS + 2];
    for (int row = 0; row < BRICK_ROWS; ++row) {
        if (!fgets(line, sizeof(line), file)) {
            break;
        }
        for (int col = 0; col < BRICK_COLS && line[col] != '\0'; ++col) {
            char brickType = line[col];
            if (brickType != ' ' && brickType != '\n' && brickType != '\r') {
                float x = col * BRICK_WIDTH;
                float y = row * BRICK_WIDTH;
                Brick* newBrick = new Brick(game->m_game_area, x, y, BRICK_WIDTH, brickType);
                game->m_bricks.push_back(newBrick);
            }
        }
    }
    fclose(file);
    return true;
}

/**
 * @brief Loads and displays a background image onto the parent object.
 */
void GameResourceManager::loadBackground(lv_obj_t* parent, const std::string& iconName) {
    const lv_draw_buf_t* bg_src = getIconSource(iconName);
    if (!bg_src) return;

    lv_obj_t* bg_img = lv_img_create(parent);
    lv_img_set_src(bg_img, bg_src); // Use draw buffer directly
    lv_obj_set_size(bg_img, GAME_AREA_WIDTH, GAME_AREA_HEIGHT);
    lv_obj_align(bg_img, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_move_background(bg_img);
}

/**
 * @brief Plays the corresponding audio based on the brick HP.
 * 
 * This function selects the appropriate `.wav` file based on the brick's HP,
 * initializes or reinitializes the audio controller, and starts audio playback.
 * If the current audio is already playing for the given brick HP, it avoids reinitialization.
 * 
 * @param brickHp The HP of the brick, used to select the correct audio file.
 * @return true if the audio started successfully, false if an error occurred.
 */
bool GameResourceManager::playAudio(int brickHp) {
    static struct timespec lastPlayTime = {0, 0};
    struct timespec now;

    clock_gettime(CLOCK_MONOTONIC, &now);

    long diffMs = (now.tv_sec - lastPlayTime.tv_sec) * 1000
                + (now.tv_nsec - lastPlayTime.tv_nsec) / 1000000;
    //Set a cooldown time to protect against excessive triggering
    const int COOLDOWN_MS = 500; 
    if (diffMs < COOLDOWN_MS) {
        return false; 
    }
    lastPlayTime = now; 

    const char* wavFile = nullptr;
    // Select appropriate audio file based on brick's HP
    switch (brickHp) {
        case -1: wavFile = "C4.wav"; break;  // For brick HP -1, play C4.wav
        case 1:  wavFile = "E4.wav"; break;  // For brick HP 1, play E4.wav
        case 2:  wavFile = "F4.wav"; break;  // For brick HP 2, play F4.wav
        case 3:  wavFile = "G4.wav"; break;  // For brick HP 3, play G4.wav
        case 4:  wavFile = "A4.wav"; break;  // For brick HP 4, play A4.wav
        case 5:  wavFile = "B4.wav"; break;  // For brick HP 5, play B4.wav
        case 6:  wavFile = "C5.wav"; break;  // For brick HP 6, play C5.wav
        default: wavFile = "C4.wav"; break;  // Default case, play C4.wav
    }

    // Build full audio file path
    std::string fullPath = getAudioPath() + "/" + wavFile;

    if (!m_audioCtl) {
        // If no audio control is initialized, initialize the audio controller
        m_audioCtl = audio_ctl_init_nxaudio(fullPath.c_str());
        audio_ctl_start(m_audioCtl);
        if (!m_audioCtl) {
            //printf("[Audio] Failed to initialize audio: %s\n", fullPath.c_str());
            return false;
        }
    } else {
        //printf("[Audio] Stopping and reinitializing audio controller...\n");
        audio_ctl_stop(m_audioCtl);
        //audio_ctl_seek(m_audioCtl, 0);
        audio_ctl_uninit_nxaudio(m_audioCtl);
        //m_audioCtl = NULL;
        m_audioCtl = audio_ctl_init_nxaudio(fullPath.c_str());
        audio_ctl_start(m_audioCtl);
    }
    return false;
}
/**
 * @brief Stops the currently playing audio and releases resources.
 * 
 * This function stops the audio if it is currently playing, uninitializes the 
 * audio controller, and frees associated resources.
 */
void GameResourceManager::stopAudio() {
    // Check if the audio controller is initialized
    if (m_audioCtl) {
       //printf("[Audio] Stopping and uninitializing audio controller...\n");

        // If audio is playing, stop it
        audio_ctl_stop(m_audioCtl);

        // Uninitialize the audio controller and clean up
        audio_ctl_uninit_nxaudio(m_audioCtl);
        m_audioCtl = nullptr;

        //printf("[Audio] Audio stopped and cleaned up.\n");
    } else {
        //printf("[Audio] No audio to stop.\n");
    }
}

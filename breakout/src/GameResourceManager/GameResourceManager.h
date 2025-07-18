#ifndef GAME_RESOURCE_MANAGER_H
#define GAME_RESOURCE_MANAGER_H

#include <string>
#include <map>
#include "lvgl.h" 
#include "Audio/audio_ctl.h"
#define RES_ROOT CONFIG_LVX_BREAKOUT_DATA_ROOT

class Game;

class GameResourceManager {
public:
    GameResourceManager();
    ~GameResourceManager();

    std::string getResPath();
    std::string getMapPath();
    std::string getFontsPath();
    std::string getIconsPath();
    std::string getAudioPath();

    bool playAudio(int brickHp);
    void stopAudio();

    bool loadMap(const std::string& mapName, Game* game);
    void loadBackground(lv_obj_t* parent, const std::string& iconName);

    const lv_draw_buf_t* getIconSource(const std::string& iconName);

    static void cleanupCache();

private:

    static std::map<std::string, lv_draw_buf_t*> s_imageCache;
    audioctl_s* m_audioCtl = nullptr;
};

#endif // GAME_RESOURCE_MANAGER_H
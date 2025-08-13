#include <stdbool.h>
#include <stdio.h> // for snprintf
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Whackmole.h"

#include "MiSans_Normal.h"
#include "MiSans_Semibold.h"
#include "grassland.h"
#include "hammer_image.h"
#include "mole_image.h"
#define GAME_TIME 30
#define MOLE_COUNT 9
static lv_obj_t* game_screen = NULL;
static lv_obj_t* game_screen_bg = NULL;
static lv_obj_t* moles[MOLE_COUNT];
static lv_obj_t* score_label;
static lv_obj_t* time_label;
static lv_obj_t* hit_effect = NULL;
static lv_obj_t* hammer_cursor = NULL;
typedef struct {
    int x_pct; // X position in percentage (0-100)
    int y_pct; // Y position in percentage (0-100)
} hole_pos_t;
// Percentage positions for holes (relative to parent size)
static const hole_pos_t hole_positions[MOLE_COUNT] = {
    { 17, 25 }, { 40, 23 }, { 67, 26 }, // Top row
    { 15, 46 }, { 43, 48 }, { 68, 46 }, // Middle row
    { 15, 68 }, { 40, 68 }, { 70, 68 } // Bottom row
};
static int score = 0;
static int game_time = GAME_TIME;
static lv_timer_t* game_timer = NULL;
static lv_timer_t* mole_timer = NULL;
static void start_game(lv_event_t* e);
static void delete_effect_cb(lv_timer_t* timer);
static void mole_click_event(lv_event_t* e);
static void update_game_timer(lv_timer_t* timer);
static void pop_random_mole(lv_timer_t* timer);
static void pointer_event_cb(lv_event_t* e);
static struct resource_s R;

static void end_msg_event_cb(lv_event_t* e)
{
    lv_obj_t* btn = lv_event_get_target(e);
    lv_obj_t* footer = lv_obj_get_parent(btn);
    lv_obj_t* mbox = lv_obj_get_parent(footer);
    lv_obj_delete(mbox);
    const char* txt = (const char*)lv_event_get_user_data(e);
    if (strcmp(txt, "Again") == 0) {
        start_game(NULL);
    }
}

static bool init_resource(void)
{
    // Styles
    lv_style_init(&R.styles.button_default);
    lv_style_init(&R.styles.button_pressed);
    lv_style_set_opa(&R.styles.button_default, LV_OPA_COVER);
    lv_style_set_opa(&R.styles.button_pressed, LV_OPA_70);

    return true;
}
// INIT
void init_whack_a_mole_game(lv_obj_t* parent)
{
    // Set random number seed
    srand(time(NULL));

    game_screen_bg = lv_image_create(parent);
    lv_image_set_src(game_screen_bg, &grassland);
    lv_obj_set_size(game_screen_bg, LV_PCT(100), LV_PCT(100));
    lv_obj_align(game_screen_bg, LV_ALIGN_CENTER, 0, 0);
    lv_image_set_inner_align(game_screen_bg, LV_IMAGE_ALIGN_STRETCH);
    lv_obj_move_background(game_screen_bg);
    lv_image_set_antialias(game_screen_bg, false);

    game_screen = lv_obj_create(parent);
    lv_obj_remove_style_all(game_screen);

    // lv_obj_set_align(game_screen, LV_ALIGN_CENTER);
    lv_obj_set_size(game_screen, lv_display_get_horizontal_resolution(NULL), lv_display_get_vertical_resolution(NULL));

    lv_obj_align(game_screen, LV_ALIGN_CENTER, 0, 0);
    lv_coord_t parent_w = lv_obj_get_style_width(game_screen, LV_PART_MAIN);
    lv_coord_t parent_h = lv_obj_get_style_height(game_screen, LV_PART_MAIN);

    // Disable scroll and click attributes
    lv_obj_remove_flag(game_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(game_screen, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(game_screen, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_remove_flag(game_screen, LV_OBJ_FLAG_SCROLL_MOMENTUM);
    lv_obj_move_foreground(game_screen);
    // Add pointer events to track mouse position
    lv_obj_add_event_cb(game_screen, pointer_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(game_screen, pointer_event_cb, LV_EVENT_PRESSING, NULL);
    static const lv_style_prop_t props[] = { LV_STYLE_TRANSFORM_ROTATION, 0 };
    /* A special transition when going to pressed state
     * Make it slow (500 ms) but start  without delay*/
    static lv_style_transition_dsc_t trans_pr;
    lv_style_transition_dsc_init(&trans_pr, props, lv_anim_path_linear, 150, 0,
        NULL);

    // // hammer cursor
    hammer_cursor = lv_image_create(game_screen);
    lv_img_set_src(hammer_cursor, &hammer_image);
    lv_obj_add_flag(hammer_cursor, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_image_recolor(hammer_cursor, lv_color_black(), 0);
    lv_obj_set_style_image_recolor_opa(hammer_cursor, LV_OPA_30, 0);
    lv_obj_set_size(hammer_cursor, parent_w / 7, parent_w / 7);
    lv_obj_get_width(hammer_cursor);
    lv_obj_get_height(hammer_cursor);
    lv_image_set_inner_align(hammer_cursor, LV_IMAGE_ALIGN_STRETCH);
    lv_image_set_antialias(hammer_cursor, false);

    lv_obj_set_style_transform_rotation(hammer_cursor, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_transform_rotation(hammer_cursor, 300, LV_STATE_PRESSED);
    lv_obj_set_style_transition(hammer_cursor, &trans_pr, LV_STATE_DEFAULT);
    lv_obj_set_style_transition(hammer_cursor, &trans_pr, LV_STATE_PRESSED);
    lv_obj_add_flag(hammer_cursor, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(hammer_cursor, LV_OBJ_FLAG_IGNORE_LAYOUT);

    lv_image_set_antialias(game_screen_bg, false);

    // title
    lv_obj_t* title = lv_label_create(game_screen);
    lv_label_set_text(title, "Whackmole");
    lv_obj_set_style_text_font(title, &MiSans_Normal, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_align(title, LV_ALIGN_TOP_MID);
    lv_obj_set_pos(title, 0, 10);

    // score display
    score_label = lv_label_create(game_screen);
    lv_label_set_text_fmt(score_label, "score: %d", score);
    lv_obj_set_style_text_font(score_label, &MiSans_Normal, 0);
    lv_obj_set_style_text_color(score_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_align(score_label, LV_ALIGN_TOP_LEFT);

    lv_obj_set_pos(score_label, 20, 10);
    // Time display
    time_label = lv_label_create(game_screen);
    lv_label_set_text_fmt(time_label, "time: %d", game_time);
    lv_obj_set_style_text_font(time_label, &MiSans_Normal, 0);
    lv_obj_set_style_text_color(time_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_align(time_label, LV_ALIGN_TOP_RIGHT);
    lv_obj_set_pos(time_label, -20, 10);

    for (int i = 0; i < 9; i++) {
        lv_obj_t* hole = lv_obj_create(game_screen);
        lv_obj_set_size(hole, LV_PCT(12), LV_PCT(20));
        // lv_obj_set_size(hole, 0.5 *parent_w, 0.5 * parent_h);
        lv_coord_t x = (hole_positions[i].x_pct * parent_w) / 100;
        lv_coord_t y = (hole_positions[i].y_pct * parent_h) / 100;
        lv_obj_set_pos(hole, x, y);
        lv_obj_set_style_border_width(hole, 0, 0);
        lv_obj_set_style_margin_all(hole, 0, 0);
        lv_obj_set_style_pad_all(hole, 0, 0);
        lv_obj_set_style_bg_opa(hole, LV_OPA_TRANSP, 0); // Set background to transparent

        lv_obj_move_background(hole);
        lv_obj_move_to_index(hole, 0);
        // create moles
        moles[i] = lv_image_create(hole);
        lv_image_set_src(moles[i], &mole_image);
        lv_obj_set_size(moles[i], LV_PCT(100), LV_PCT(100));
        lv_image_set_inner_align(moles[i], LV_IMAGE_ALIGN_STRETCH);
        // lv_obj_set_style_border_width(moles[i], 5, 0);
        lv_obj_set_style_margin_all(moles[i], 0, 0);
        lv_obj_set_style_pad_all(moles[i], 0, 0);
        // lv_image_set_scale(moles[i], 200);
        lv_obj_set_align(moles[i], LV_ALIGN_CENTER);

        lv_obj_add_flag(moles[i], LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(moles[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_move_foreground(moles[i]);

        // click event
        lv_obj_add_event_cb(moles[i], mole_click_event, LV_EVENT_CLICKED, NULL);
        // Hover event
        lv_obj_add_event_cb(moles[i], pointer_event_cb, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(moles[i], pointer_event_cb, LV_EVENT_PRESSING, NULL);
        lv_obj_add_event_cb(moles[i], pointer_event_cb, LV_EVENT_RELEASED, NULL);
    }
    // Update hole positions based on current screen size
    // start button
    lv_obj_t* start_btn = lv_btn_create(game_screen);
    lv_obj_set_size(start_btn, 120, 40);
    lv_obj_set_align(start_btn, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_pos(start_btn, 0, -20);
    lv_obj_add_event_cb(start_btn, start_game, LV_EVENT_CLICKED, start_btn);
    lv_obj_set_style_bg_color(start_btn, lv_color_hex(0x228B22), 0);
    lv_obj_t* btn_label = lv_label_create(start_btn);
    lv_label_set_text(btn_label, "START");
    lv_obj_set_style_text_font(btn_label, &MiSans_Normal, 0);
    lv_obj_set_style_text_color(start_btn, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(btn_label);
    lv_obj_move_foreground(hammer_cursor);
    lv_obj_move_foreground(start_btn);
    lv_obj_move_foreground(score_label);
    lv_obj_move_foreground(time_label);
    lv_obj_move_foreground(title);
}
static void pointer_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_point_t pos;
    lv_indev_t* indev = lv_indev_active();
    if (indev == NULL)
        return;
    lv_indev_get_point(indev, &pos);
    lv_obj_remove_flag(hammer_cursor, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(hammer_cursor, pos.x - 25, pos.y - 25);

    if (code == LV_EVENT_PRESSED) {
        lv_obj_set_state(hammer_cursor, LV_STATE_PRESSED, true);
    } else if (code == LV_EVENT_PRESSING) {
    } else {
        lv_obj_set_state(hammer_cursor, LV_STATE_PRESSED, false);
    }
}

// start game
static void start_game(lv_event_t* e)
{
    score = 0;
    game_time = GAME_TIME;
    if (e != NULL) {
        lv_obj_t* start_btn = lv_event_get_user_data(e);
        lv_obj_delete(start_btn);
    }

    printf("Game started! Score: %d, Time: %d\n", score, game_time);
    lv_label_set_text_fmt(score_label, "score: %d", score);
    lv_label_set_text_fmt(time_label, "time: %d", game_time);
    lv_obj_set_style_text_font(score_label, &MiSans_Normal, 0);
    lv_obj_set_style_text_color(game_screen, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(time_label, &MiSans_Normal, 0);
    lv_obj_set_style_text_color(game_screen, lv_color_hex(0xFFFFFF), 0);

    for (int i = 0; i < 9; i++) {
        lv_obj_add_flag(moles[i], LV_OBJ_FLAG_HIDDEN);
    }
    if (game_timer) {
        lv_timer_delete(game_timer);
        game_timer = NULL;
    }
    game_timer = lv_timer_create(update_game_timer, 1000, NULL);

    if (mole_timer) {
        lv_timer_delete(mole_timer);
        mole_timer = NULL;
    }
    mole_timer = lv_timer_create(pop_random_mole, 1000, NULL);
}

// Update the game timer
static void update_game_timer(lv_timer_t* timer)
{
    game_time--;
    lv_label_set_text_fmt(time_label, "time: %d", game_time);
    if (game_time <= 0) {
        lv_timer_delete(game_timer);
        game_timer = NULL;

        if (mole_timer) {
            lv_timer_delete(mole_timer);
            mole_timer = NULL;
        }
        for (int i = 0; i < 9; i++) {
            lv_obj_add_flag(moles[i], LV_OBJ_FLAG_HIDDEN);
        }
        lv_obj_t* end_msg = lv_msgbox_create(lv_screen_active());
        lv_msgbox_add_title(end_msg, "Game over!");
        lv_msgbox_add_text(end_msg, "Congratulations on completing game!");
        lv_obj_set_size(end_msg, 300, 120);
        lv_msgbox_add_close_button(end_msg);
        const char* btns[] = { "Again", NULL };
        for (int i = 0; btns[i]; i++) {
            lv_obj_t* btn = lv_msgbox_add_footer_button(end_msg, btns[i]);
            lv_obj_set_style_text_font(btn, &MiSans_Normal, 0);

            lv_obj_add_event_cb(btn, end_msg_event_cb, LV_EVENT_CLICKED,
                (char*)btns[i]);
        }
        return;
    }
}

// Gophers appear randomly
static void pop_random_mole(lv_timer_t* timer)
{
    for (int i = 0; i < 9; i++) {
        lv_obj_add_flag(moles[i], LV_OBJ_FLAG_HIDDEN);
    }
    int show_count = rand() % 2 + 1;
    for (int i = 0; i < show_count; i++) {
        int mole_idx = rand() % 9;
        lv_obj_remove_flag(moles[mole_idx], LV_OBJ_FLAG_HIDDEN);
    }

    // Adjust the frequency of gophers
    if (game_time < 40) {
        lv_timer_set_period(timer, 800);
    }
    if (game_time < 20) {
        lv_timer_set_period(timer, 600);
    }
}

// Gopher click events
static void mole_click_event(lv_event_t* e)
{
    lv_obj_t* mole = lv_event_get_target(e);
    LV_LOG_USER("clicked %d", lv_obj_has_flag(mole, LV_OBJ_FLAG_HIDDEN));
    // Bonus points when the gopher is visible
    if (!lv_obj_has_flag(mole, LV_OBJ_FLAG_HIDDEN)) {
        score++;
        lv_label_set_text_fmt(score_label, "score: %d", score);
        lv_obj_add_flag(mole, LV_OBJ_FLAG_HIDDEN);
        if (hit_effect != NULL) {
            lv_obj_delete(hit_effect);
            hit_effect = NULL;
        }
        hit_effect = lv_obj_create(lv_obj_get_parent(mole));
        lv_obj_set_size(hit_effect, LV_PCT(100), LV_PCT(100));
        lv_obj_set_align(hit_effect, LV_ALIGN_CENTER);
        lv_obj_set_pos(hit_effect, 0, 0);
        lv_obj_set_style_bg_color(hit_effect, lv_color_hex(0xFFFF00), 0);
        lv_obj_set_style_bg_opa(hit_effect, LV_OPA_50, 0);
        lv_timer_create(delete_effect_cb, 300, hit_effect);
    }
}

// Delete click effects
static void delete_effect_cb(lv_timer_t* timer)
{
    lv_obj_t* effect = timer->user_data;
    if (effect) {
        lv_obj_delete(effect);
        hit_effect = NULL;
    }
    lv_timer_delete(timer);
}

// create app
void whackmole_app_create(void)
{
    LV_FONT_DECLARE(MiSans_Normal);
    lv_obj_t* scr = lv_screen_active();
    init_resource();
    init_whack_a_mole_game(scr);
}

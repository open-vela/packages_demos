/* UI
 *
 * Resource
 *     init_resource(): Load fonts, images, and other resources required for the game
 *
 * Main Page
 *     app_create_main_page(): Creates the main chessboard page
 *     app_create_error_page(): Creates an error page if resources fail
 *
 * Chess Board
 *     position_init(): Initialize chessboard and pieces positions
 *     piece_position(x, y): Get or set piece positions
 *     piece_change(changestrcut* p): Update piece positions after a move
 *     piece_information_change(changestrcut* p_informain): Update piece info like type or camp
 *     reset(): Reset chessboard for a new game
 *     board_check(): Check board state for illegal moves
 *     board_check_shuai_statue(): Check general/king status
 *     reset_choose_page(choose_camp): Reset camp selection UI
 *
 * Piece Rules
 *     car(x1, y1, x2, y2, Camp): Valid move check for rook
 *     horse(x1, y1, x2, y2, Camp): Valid move check for knight
 *     elephant(x1, y1, x2, y2, Camp): Valid move check for elephant/minister
 *     shi(x1, y1, x2, y2, Camp): Valid move check for advisor/guard
 *     jiang(x1, y1, x2, y2, Camp): Valid move check for general/king
 *     pao(x1, y1, x2, y2, Camp): Valid move check for cannon
 *     bing(x1, y1, x2, y2, Camp): Valid move check for soldier/pawn
 *     zu(x1, y1, x2, y2, Camp): Valid move check for other camp’s soldier/pawn
 *
 * Event Handlers
 *     app_get_point_event_handler(lv_event_t* e): Handle player clicks on board
 *     restart_event_cb(lv_event_t* e): Handle restart button
 *     exit_event_cb(lv_event_t* e): Handle exit button
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "Chinese_chess.h"
#include "stdlib.h"
#include "time.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
/* Resource initialization and UI pages */
static bool init_resource(void);
static void app_create_main_page(void);
static void app_create_error_page(void);
/* Chess piece movement functions */
static int car(int x1, int y1, int x2, int y2, int Camp);
static int horse(int x1, int y1, int x2, int y2, int Camp);
static int elephant(int x1, int y1, int x2, int y2, int Camp);
static int shi(int x1, int y1, int x2, int y2, int Camp);
static int jiang(int x1, int y1, int x2, int y2, int Camp);
static int pao(int x1, int y1, int x2, int y2, int Camp);
static int bing(int x1, int y1, int x2, int y2, int Camp);
static int zu(int x1, int y1, int x2, int y2, int Camp);
/* Chessboard management */
static void position_init(void);
static void piece_position(long int x, long int y);
static void piece_change(changestrcut* p);
static void piece_information_change(changestrcut* p_informain);
static void reset(void);
static int board_check(void);
static int board_check_shuai_statue(void);
static void reset_choose_page(int choose_camp);
/* Event handlers */
static void app_get_point_event_handler(lv_event_t* e);
static void restart_event_cb(lv_event_t* e);
static void exit_event_cb(lv_event_t* e);

/**********************
 *      TYPEDEFS
 **********************/

static struct {
    const char* chessboard;
    const char* redcar;
    const char* redhorse;
    const char* redelephant;
    const char* redshi;
    const char* redjiang;
    const char* redpao;
    const char* redbing;
    const char* blackcar;
    const char* blackhorse;
    const char* blackelephant;
    const char* blackshi;
    const char* blackjiang;
    const char* blackpao;
    const char* blackbing;
} images;
pieceStruct piece[32] = {
    { "redcar1", 0, 0, redCamp, car, alive },
    { "redhorse1", 1, 0, redCamp, horse, alive },
    { "redelephant1", 2, 0, redCamp, elephant, alive },
    { "redshi1", 3, 0, redCamp, shi, alive },
    { "redjiang1", 4, 0, redCamp, jiang, alive },
    { "redshi2", 5, 0, redCamp, shi, alive },
    { "redelephant2", 6, 0, redCamp, elephant, alive },
    { "redhorse2", 7, 0, redCamp, horse, alive },
    { "redcar2", 8, 0, redCamp, car, alive },
    { "redpao1", 1, 2, redCamp, pao, alive },
    { "redpao2", 7, 2, redCamp, pao, alive },
    { "redbing1", 0, 3, redCamp, bing, alive },
    { "redbing2", 2, 3, redCamp, bing, alive },
    { "redbing3", 4, 3, redCamp, bing, alive },
    { "redbing4", 6, 3, redCamp, bing, alive },
    { "redbing5", 8, 3, redCamp, bing, alive },
    { "blackcar1", 0, 9, blackCamp, car, alive },
    { "blackhorse1", 1, 9, blackCamp, horse, alive },
    { "blackelephant1", 2, 9, blackCamp, elephant, alive },
    { "blackshi1", 3, 9, blackCamp, shi, alive },
    { "blackjiang1", 4, 9, blackCamp, jiang, alive },
    { "blackshi2", 5, 9, blackCamp, shi, alive },
    { "blackelephant2", 6, 9, blackCamp, elephant, alive },
    { "blackhorse2", 7, 9, blackCamp, horse, alive },
    { "blackcar2", 8, 9, blackCamp, car, alive },
    { "blackpao1", 1, 7, blackCamp, pao, alive },
    { "blackpao2", 7, 7, blackCamp, pao, alive },
    { "blackzu1", 0, 6, blackCamp, zu, alive },
    { "blackzu2", 2, 6, blackCamp, zu, alive },
    { "blackzu3", 4, 6, blackCamp, zu, alive },
    { "blackzu4", 6, 6, blackCamp, zu, alive },
    { "blackzu5", 8, 6, blackCamp, zu, alive }
};

pieceStruct1 piece_struct[32] = {
    { "redcar1", 0, 0, redCamp },
    { "redhorse1", 1, 0, redCamp },
    { "redelephant1", 2, 0, redCamp },
    { "redshi1", 3, 0, redCamp },
    { "redjiang", 4, 0, redCamp },
    { "redshi2", 5, 0, redCamp },
    { "redelephant2", 6, 0, redCamp },
    { "redhorse2", 7, 0, redCamp },
    { "redcar2", 8, 0, redCamp },
    { "redpao1", 1, 2, redCamp },
    { "redpao2", 7, 2, redCamp },
    { "redbing1", 0, 3, redCamp },
    { "redbing2", 2, 3, redCamp },
    { "redbing3", 4, 3, redCamp },
    { "redbing4", 6, 3, redCamp },
    { "redbing5", 8, 3, redCamp },
    { "blackcar1", 0, 9, blackCamp },
    { "blackhorse1", 1, 9, blackCamp },
    { "blackelephant1", 2, 9, blackCamp },
    { "blackshi1", 3, 9, blackCamp },
    { "blackjiang1", 4, 9, blackCamp },
    { "blackshi2", 5, 9, blackCamp },
    { "blackelephant2", 6, 9, blackCamp },
    { "blackhorse2", 7, 9, blackCamp },
    { "blackcar2", 8, 9, blackCamp },
    { "blackpao1", 1, 7, blackCamp },
    { "blackpao2", 7, 7, blackCamp },
    { "blackzu1", 0, 6, blackCamp },
    { "blackzu2", 2, 6, blackCamp },
    { "blackzu3", 4, 6, blackCamp },
    { "blackzu4", 6, 6, blackCamp },
    { "blackzu5", 8, 6, blackCamp }
};

changestrcut changebuffer = {
    0, 0, 0, 0, 0, 0
};

/*********************
 *  STATIC VARIABLES
 *********************/

/*Instantiate chess pieces*/
static lv_obj_t* Red_car_1;
static lv_obj_t* Red_car_2;
static lv_obj_t* Red_horse_1;
static lv_obj_t* Red_horse_2;
static lv_obj_t* Red_elephant_1;
static lv_obj_t* Red_elephant_2;
static lv_obj_t* Red_shi_1;
static lv_obj_t* Red_shi_2;
static lv_obj_t* Red_jiang;
static lv_obj_t* Red_pao_1;
static lv_obj_t* Red_pao_2;
static lv_obj_t* Red_bing_1;
static lv_obj_t* Red_bing_2;
static lv_obj_t* Red_bing_3;
static lv_obj_t* Red_bing_4;
static lv_obj_t* Red_bing_5;
static lv_obj_t* Black_car_1;
static lv_obj_t* Black_car_2;
static lv_obj_t* Black_horse_1;
static lv_obj_t* Black_horse_2;
static lv_obj_t* Black_elephant_1;
static lv_obj_t* Black_elephant_2;
static lv_obj_t* Black_shi_1;
static lv_obj_t* Black_shi_2;
static lv_obj_t* Black_jiang;
static lv_obj_t* Black_pao_1;
static lv_obj_t* Black_pao_2;
static lv_obj_t* Black_bing_1;
static lv_obj_t* Black_bing_2;
static lv_obj_t* Black_bing_3;
static lv_obj_t* Black_bing_4;
static lv_obj_t* Black_bing_5;

/*Define the chessboard coordinate position*/
static int Board_Xvalue[9] = {
    342, 416, 492, 567, 641, 715, 792, 867, 938
};
static int Board_Yvalue[10] = {
    63, 140, 216, 289, 365, 437, 514, 591, 665, 737
};

static int click_number = 0; // The chessboard is 0 when not clicked, and increments by 1 upon clicking
static int chess, chess2;
static int turn_camp = 1;

static int chess_board[10][9] = {
    { red_che_1, red_ma_1, red_xiang_1, red_shi_1, red_jiang, red_shi_2, red_xiang_2, red_ma_2, red_che_2 },
    { board_black, board_black, board_black, board_black, board_black, board_black, board_black, board_black, board_black },
    { board_black, red_pao_1, board_black, board_black, board_black, board_black, board_black, red_pao_2, board_black },
    { red_bing_1, board_black, red_bing_2, board_black, red_bing_3, board_black, red_bing_4, board_black, red_bing_5 },
    { board_black, board_black, board_black, board_black, board_black, board_black, board_black, board_black, board_black },
    { board_black, board_black, board_black, board_black, board_black, board_black, board_black, board_black, board_black },
    { black_bing_1, board_black, black_bing_2, board_black, black_bing_3, board_black, black_bing_4, board_black, black_bing_5 },
    { board_black, black_pao_1, board_black, board_black, board_black, board_black, board_black, black_pao_2, board_black },
    { board_black, board_black, board_black, board_black, board_black, board_black, board_black, board_black, board_black },
    { black_che_1, black_ma_1, black_xiang_1, black_shi_1, black_jiang, black_shi_2, black_xiang_2, black_ma_2, black_che_2 }
};

/*********************
 *   GLOBAL FUNCTIONS
 *********************/

void Chinese_Chess_app_create(void)
{
    // lv_memzero(&R, sizeof(R));

    bool resource_healthy_check;
    resource_healthy_check = init_resource();

    if (!resource_healthy_check) {
        app_create_error_page();
        return;
    }

    app_create_main_page();
}

static void app_create_main_page(void)
{
    position_init();
    lv_obj_add_event_cb(lv_scr_act(), app_get_point_event_handler, LV_EVENT_ALL, NULL);
}

static void position_init(void)
{
    lv_obj_t* root = lv_screen_active();
    lv_obj_set_style_bg_img_src(root, images.chessboard, LV_PART_MAIN); // Chessboard background
    lv_obj_set_style_border_width(root, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(root, 0, LV_PART_MAIN);
    // Add red car
    Red_car_1 = lv_image_create(root);
    lv_image_set_src(Red_car_1, images.redcar);
    lv_obj_set_pos(Red_car_1, Board_Xvalue[piece[red_che_1].Xvalue] - 35, Board_Yvalue[piece[red_che_1].Yvalue] - 35);
    Red_car_2 = lv_image_create(root);
    lv_image_set_src(Red_car_2, images.redcar);
    lv_obj_set_pos(Red_car_2, Board_Xvalue[piece[red_che_2].Xvalue] - 35, Board_Yvalue[piece[red_che_2].Yvalue] - 35);
    // Add red horse
    Red_horse_1 = lv_image_create(root);
    lv_image_set_src(Red_horse_1, images.redhorse);
    lv_obj_set_pos(Red_horse_1, Board_Xvalue[piece[red_ma_1].Xvalue] - 35, Board_Yvalue[piece[red_ma_1].Yvalue] - 35);
    Red_horse_2 = lv_image_create(root);
    lv_image_set_src(Red_horse_2, images.redhorse);
    lv_obj_set_pos(Red_horse_2, Board_Xvalue[piece[red_ma_2].Xvalue] - 35, Board_Yvalue[piece[red_ma_2].Yvalue] - 35);
    // Add red elephant
    Red_elephant_1 = lv_image_create(root);
    lv_image_set_src(Red_elephant_1, images.redelephant);
    lv_obj_set_pos(Red_elephant_1, Board_Xvalue[piece[red_xiang_1].Xvalue] - 35, Board_Yvalue[piece[red_xiang_1].Yvalue] - 35);
    Red_elephant_2 = lv_image_create(root);
    lv_image_set_src(Red_elephant_2, images.redelephant);
    lv_obj_set_pos(Red_elephant_2, Board_Xvalue[piece[red_xiang_2].Xvalue] - 35, Board_Yvalue[piece[red_xiang_2].Yvalue] - 35);
    // Add red shi
    Red_shi_1 = lv_image_create(root);
    lv_image_set_src(Red_shi_1, images.redshi);
    lv_obj_set_pos(Red_shi_1, Board_Xvalue[piece[red_shi_1].Xvalue] - 35, Board_Yvalue[piece[red_shi_1].Yvalue] - 35);
    Red_shi_2 = lv_image_create(root);
    lv_image_set_src(Red_shi_2, images.redshi);
    lv_obj_set_pos(Red_shi_2, Board_Xvalue[piece[red_shi_2].Xvalue] - 35, Board_Yvalue[piece[red_shi_2].Yvalue] - 35);
    // Add red general
    Red_jiang = lv_image_create(root);
    lv_image_set_src(Red_jiang, images.redjiang);
    lv_obj_set_pos(Red_jiang, Board_Xvalue[piece[red_jiang].Xvalue] - 35, Board_Yvalue[piece[red_jiang].Yvalue] - 35);
    // Add red pao
    Red_pao_1 = lv_image_create(root);
    lv_image_set_src(Red_pao_1, images.redpao);
    lv_obj_set_pos(Red_pao_1, Board_Xvalue[piece[red_pao_1].Xvalue] - 35, Board_Yvalue[piece[red_pao_1].Yvalue] - 35);
    Red_pao_2 = lv_image_create(root);
    lv_image_set_src(Red_pao_2, images.redpao);
    lv_obj_set_pos(Red_pao_2, Board_Xvalue[piece[red_pao_2].Xvalue] - 35, Board_Yvalue[piece[red_pao_2].Yvalue] - 35);
    // Add red soldiers
    Red_bing_1 = lv_image_create(root);
    lv_image_set_src(Red_bing_1, images.redbing);
    lv_obj_set_pos(Red_bing_1, Board_Xvalue[piece[red_bing_1].Xvalue] - 35, Board_Yvalue[piece[red_bing_1].Yvalue] - 35);
    Red_bing_2 = lv_image_create(root);
    lv_image_set_src(Red_bing_2, images.redbing);
    lv_obj_set_pos(Red_bing_2, Board_Xvalue[piece[red_bing_2].Xvalue] - 35, Board_Yvalue[piece[red_bing_2].Yvalue] - 35);
    Red_bing_3 = lv_image_create(root);
    lv_image_set_src(Red_bing_3, images.redbing);
    lv_obj_set_pos(Red_bing_3, Board_Xvalue[piece[red_bing_3].Xvalue] - 35, Board_Yvalue[piece[red_bing_3].Yvalue] - 35);
    Red_bing_4 = lv_image_create(root);
    lv_image_set_src(Red_bing_4, images.redbing);
    lv_obj_set_pos(Red_bing_4, Board_Xvalue[piece[red_bing_4].Xvalue] - 35, Board_Yvalue[piece[red_bing_4].Yvalue] - 35);
    Red_bing_5 = lv_image_create(root);
    lv_image_set_src(Red_bing_5, images.redbing);
    lv_obj_set_pos(Red_bing_5, Board_Xvalue[piece[red_bing_5].Xvalue] - 35, Board_Yvalue[piece[red_bing_5].Yvalue] - 35);
    /*Create a black chess faction, which includes instantiating chess pieces, transferring images, and locating chess pieces*/
    // Instantiate chess pieces
    // Instantiate black car
    Black_car_1 = lv_image_create(root);
    Black_car_2 = lv_image_create(root);
    // Instantiate black horse
    Black_horse_1 = lv_image_create(root);
    Black_horse_2 = lv_image_create(root);
    // Instantiate black pao
    Black_pao_1 = lv_image_create(root);
    Black_pao_2 = lv_image_create(root);
    // Instantiate black elephant
    Black_elephant_1 = lv_image_create(root);
    Black_elephant_2 = lv_image_create(root);
    // Instantiate black shi
    Black_shi_1 = lv_image_create(root);
    Black_shi_2 = lv_image_create(root);
    // Instantiate black general
    Black_jiang = lv_image_create(root);
    // Instantiate black soldiers
    Black_bing_1 = lv_image_create(root);
    Black_bing_2 = lv_image_create(root);
    Black_bing_3 = lv_image_create(root);
    Black_bing_4 = lv_image_create(root);
    Black_bing_5 = lv_image_create(root);
    // Send pictures
    lv_image_set_src(Black_car_1, images.blackcar);
    lv_image_set_src(Black_car_2, images.blackcar);
    lv_image_set_src(Black_horse_1, images.blackhorse);
    lv_image_set_src(Black_horse_2, images.blackhorse);
    lv_image_set_src(Black_pao_1, images.blackpao);
    lv_image_set_src(Black_pao_2, images.blackpao);
    lv_image_set_src(Black_jiang, images.blackjiang);
    lv_image_set_src(Black_elephant_1, images.blackelephant);
    lv_image_set_src(Black_elephant_2, images.blackelephant);
    lv_image_set_src(Black_shi_1, images.blackshi);
    lv_image_set_src(Black_shi_2, images.blackshi);
    lv_image_set_src(Black_bing_1, images.blackbing);
    lv_image_set_src(Black_bing_2, images.blackbing);
    lv_image_set_src(Black_bing_3, images.blackbing);
    lv_image_set_src(Black_bing_4, images.blackbing);
    lv_image_set_src(Black_bing_5, images.blackbing);
    // Locate the coordinates of the chess piece
    lv_obj_set_pos(Black_car_1, Board_Xvalue[piece[black_che_1].Xvalue] - 35, Board_Yvalue[piece[black_che_1].Yvalue] - 35);
    lv_obj_set_pos(Black_car_2, Board_Xvalue[piece[black_che_2].Xvalue] - 35, Board_Yvalue[piece[black_che_2].Yvalue] - 35);
    lv_obj_set_pos(Black_horse_1, Board_Xvalue[piece[black_ma_1].Xvalue] - 35, Board_Yvalue[piece[black_ma_1].Yvalue] - 35);
    lv_obj_set_pos(Black_horse_2, Board_Xvalue[piece[black_ma_2].Xvalue] - 35, Board_Yvalue[piece[black_ma_2].Yvalue] - 35);
    lv_obj_set_pos(Black_pao_1, Board_Xvalue[piece[black_pao_1].Xvalue] - 35, Board_Yvalue[piece[black_pao_1].Yvalue] - 35);
    lv_obj_set_pos(Black_pao_2, Board_Xvalue[piece[black_pao_2].Xvalue] - 35, Board_Yvalue[piece[black_pao_2].Yvalue] - 35);
    lv_obj_set_pos(Black_elephant_1, Board_Xvalue[piece[black_xiang_1].Xvalue] - 35, Board_Yvalue[piece[black_xiang_1].Yvalue] - 35);
    lv_obj_set_pos(Black_elephant_2, Board_Xvalue[piece[black_xiang_2].Xvalue] - 35, Board_Yvalue[piece[black_xiang_2].Yvalue] - 35);
    lv_obj_set_pos(Black_shi_1, Board_Xvalue[piece[black_shi_1].Xvalue] - 35, Board_Yvalue[piece[black_shi_1].Yvalue] - 35);
    lv_obj_set_pos(Black_shi_2, Board_Xvalue[piece[black_shi_2].Xvalue] - 35, Board_Yvalue[piece[black_shi_2].Yvalue] - 35);
    lv_obj_set_pos(Black_jiang, Board_Xvalue[piece[black_jiang].Xvalue] - 35, Board_Yvalue[piece[black_jiang].Yvalue] - 35);
    lv_obj_set_pos(Black_bing_1, Board_Xvalue[piece[black_bing_1].Xvalue] - 35, Board_Yvalue[piece[black_bing_1].Yvalue] - 35);
    lv_obj_set_pos(Black_bing_2, Board_Xvalue[piece[black_bing_2].Xvalue] - 35, Board_Yvalue[piece[black_bing_2].Yvalue] - 35);
    lv_obj_set_pos(Black_bing_3, Board_Xvalue[piece[black_bing_3].Xvalue] - 35, Board_Yvalue[piece[black_bing_3].Yvalue] - 35);
    lv_obj_set_pos(Black_bing_4, Board_Xvalue[piece[black_bing_4].Xvalue] - 35, Board_Yvalue[piece[black_bing_4].Yvalue] - 35);
    lv_obj_set_pos(Black_bing_5, Board_Xvalue[piece[black_bing_5].Xvalue] - 35, Board_Yvalue[piece[black_bing_5].Yvalue] - 35);
}

static void app_get_point_event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_PRESSED) {
        lv_point_t p;
        lv_indev_t* indev = lv_indev_get_act();
        lv_indev_get_point(indev, &p);
        piece_position(p.x, p.y);
    }
}

// Loading resources such as images
static bool init_resource(void)
{
    // images
    images.chessboard = ICONS_ROOT "/Board.png";
    images.blackbing = ICONS_ROOT "/blackzu.png";
    images.redcar = ICONS_ROOT "/redcar.png";
    images.redhorse = ICONS_ROOT "/redhorse.png";
    images.redelephant = ICONS_ROOT "/redelephant.png";
    images.redshi = ICONS_ROOT "/redshi.png";
    images.redjiang = ICONS_ROOT "/redjiang.png";
    images.redpao = ICONS_ROOT "/redpao.png";
    images.redbing = ICONS_ROOT "/redbing.png";
    images.blackcar = ICONS_ROOT "/blackcar.png";
    images.blackjiang = ICONS_ROOT "/blackjiang.png";
    images.blackhorse = ICONS_ROOT "/blackhorse.png";
    images.blackpao = ICONS_ROOT "/blackpao.png";
    images.blackshi = ICONS_ROOT "/blackshi.png";
    images.blackelephant = ICONS_ROOT "/blackelephant.png";
    return true;
}

// Chess piece function
static int car(int x1, int y1, int x2, int y2, int Camp)
{
    int index, temp, i = 1;
    if (x1 != x2 && y1 != y2) {
        // Cars can only travel horizontally or vertically
        return 0;
    }
    // Obtain the chessboard information of the target location
    index = chess_board[y2][x2];

    if (index != board_black && piece[index].camp == Camp) {
        // If there are own chess pieces at the target location, they cannot be moved
        return 0;
    }

    if (x1 == x2) {
        // vertical movement
        if (y1 > y2) {
            // If y1>y2, swap the two values to ensure that y2 is the larger one
            temp = y1;
            y1 = y2;
            y2 = temp;
        }
        while (chess_board[y1 + i][x1] == board_black) {
            // Determine if there are any other chess pieces between y1 and y2, and if there are, they cannot be moved
            if (y1 + i == y2)
                break;
            i++;
        }
        if (y1 + i != y2) {
            // There is a chess piece obstructing in the middle
            return 0;
        }

    } else {
        // lateral movement
        if (x1 > x2) {
            temp = x1;
            x1 = x2;
            x2 = temp;
        }
        while (chess_board[y1][x1 + i] == board_black) {
            if (x1 + i == x2)
                break;
            i++;
        }
        if (x1 + i != x2) {
            // There is a chess piece obstructing in the middle
            return 0;
        }
    }
    // No obstacles, the chess pieces can move to the target location normally
    return 1;
}

static int horse(int x1, int y1, int x2, int y2, int Camp)
{
    int dx, dy, x3, y3, index;
    dx = x2 - x1;
    dy = y2 - y1;
    if (dx * dy != 2 && dx * dy != -2) {
        // Horse moves like the sun
        return 0;
    }
    index = chess_board[y2][x2];
    if (index != board_black && piece[index].camp == Camp) {
        // The destination has our own chess pieces and cannot be moved
        return 0;
    }
    x3 = x1 + dx / 2;
    y3 = y1 + dy / 2;
    if (chess_board[y3][x3] != board_black) {
        return 0;
    }
    if (x2 < 0 || y2 < 0 || x2 > 8 || y2 > 9) {
        // Don't act recklessly
        return 0;
    }
    // No obstacles, the chess pieces can move to the target location normally
    return 1;
}

static int elephant(int x1, int y1, int x2, int y2, int Camp)
{
    int index, dx, dy;
    dx = x2 - x1;
    dy = y2 - y1;

    if ((dx != 2 && dx != -2) || (dy != 2 && dy != -2)) {
        // The distance of movement must be 2
        return 0;
    }

    index = chess_board[y1 + dy / 2][x1 + dx / 2]; // Check if there are any chess pieces at the elephant's feet here. If there are, they cannot be moved
    if (index != -1) {
        // The elephant's eyes are blocked
        return 0;
    }
    if (x2 < 0 || y2 < 0 || x2 > 8 || y2 > 9)
        return 0;
    if (y1 < 5) {
        if (y2 > 4) {
            // Elephants cannot cross the river
            return 0;
        }

    } else {
        if (y2 < 5) {
            // Elephants cannot cross the river
            return 0;
        }
    }
    index = chess_board[y2][x2];
    if (index != -1 && piece[index].camp == Camp) // The destination has our own chess pieces and cannot be moved
        return 0;
    // No obstacles, the chess pieces can move to the target location normally
    return 1;
}

static int shi(int x1, int y1, int x2, int y2, int Camp)
{
    int index, dx, dy;
    dx = x2 - x1;
    dy = y2 - y1;

    if (x2 < 3 || x2 > 5) {
        // Shi can only walk in the fields
        return 0;
    }
    if (dx * dx + dy * dy != 2) {
        // Shi walked diagonally
        return 0;
    }
    if (y1 < 5) {
        if (y2 > 2 || y2 < 0) {
            // Shi can only walk in the fields
            return 0;
        }
    } else {
        if (y2 < 7 || y2 > 9) {
            // Shi walked diagonally
            return 0;
        }
    }
    index = chess_board[y2][x2];
    if (index != board_black && piece[index].camp == Camp)
        return 0;
    // No obstacles, the chess pieces can move to the target location normally
    return 1;
}

static int jiang(int x1, int y1, int x2, int y2, int Camp)
{
    int index, dx, dy;
    dx = x2 - x1;
    dy = y2 - y1;
    index = chess_board[y2][x2];
    if (index != -1 && piece[index].camp == Camp)
        return 0;

    if (x2 < 3 || x2 > 5) {
        // Handsome can only walk in the fields
        return 0;
    }
    if (dx * dx + dy * dy != 1) {
        // Handsome, only walk one step at a time
        return 0;
    }
    if (y1 < 5) {
        if (y2 > 2 || y2 < 0) {
            // Handsome can only walk in the fields
            return 0;
        }
    } else {
        if (y2 < 7 || y2 > 9) {
            // Handsome can only walk in the fields
            return 0;
        }
    }
    // No obstacles, the chess pieces can move to the target location normally
    return 1;
}

static int pao(int x1, int y1, int x2, int y2, int Camp)
{
    int index, i, temp, pieceN, temp1;
    temp1 = chess_board[y2][x2];
    pieceN = 0;
    if (x1 != x2 && y1 != y2) // If they are not on the same straight line, they cannot be moved
        return 0;
    if (x1 == x2 && y1 == y2) // Move in place
        return 0;
    if (x2 < 0 || y2 < 0 || x2 > 8 || y2 > 9)
        return 0;
    index = chess_board[y2][x2];
    i = 1;
    if (x1 == x2) {
        // vertical movement
        if (y1 > y2) {
            temp = y1;
            y1 = y2;
            y2 = temp;
        }
        for (i = 1; y1 + i < y2; i++) {
            // Polling all positions between y1 and y2 (excluding y1 and y2) once
            if (chess_board[y1 + i][x1] != board_black) // If there are chess pieces, piece N++is used to count the number of pieces between two positions
                pieceN++;
        }
    } else {
        // lateral movement
        if (x1 > x2) {
            temp = x1;
            x1 = x2;
            x2 = temp;
        }
        for (i = 1; x1 + i < x2; i++) {
            if (chess_board[y1][x1 + i] != board_black)
                pieceN++;
        }
    }

    if (pieceN > 1) // At this point, pieceN represents the number of chess pieces between the source and destination positions
        return 0;

    if ((pieceN == 1 && temp1 == -1) || (pieceN == 0 && temp1 != -1))
        return 0;

    if (index != -1 && piece[index].camp == Camp)
        return 0;

    // No obstacles, the chess pieces can move to the target location normally
    return 1;
}

static int bing(int x1, int y1, int x2, int y2, int Camp)
{
    int index, dx, dy;
    dx = x2 - x1;
    dy = y2 - y1;
    if (dy * dy + dx * dx != 1)
        return 0;
    if (x2 < 0 || y2 < 0 || x2 > 8 || y2 > 9)
        return 0;
    if (piece[red_bing_1].Yvalue < 5) {
        if (dy < 0) // Soldiers cannot retreat
            return 0;
        if (y1 <= 4 && dx != 0) // The soldiers did not cross the river
            return 0;
    } else {
        if (dy > 0)
            return 0;
        if (y1 >= 5 && dx != 0)
            return 0;
    }
    index = chess_board[y2][x2];
    if (index != -1 && piece[index].camp == Camp)
        return 0;
    // No obstacles, the chess pieces can move to the target location normally
    return 1;
}

static int zu(int x1, int y1, int x2, int y2, int Camp)
{
    int index, dx, dy;
    dx = x2 - x1;
    dy = y2 - y1;
    if (dy * dy + dx * dx != 1)
        return 0;
    if (x2 < 0 || y2 < 0 || x2 > 8 || y2 > 9)
        return 0;
    if (piece[black_bing_1].Yvalue < 5) {
        if (dy < 0)
            return 0;
        if (y1 <= 4 && dx != 0)
            return 0;
    } else {
        if (dy > 0)
            return 0;
        if (y1 >= 5 && dx != 0)
            return 0;
    }
    index = chess_board[y2][x2];
    if (index != -1 && piece[index].camp == Camp)
        return 0;
    // No obstacles, the chess pieces can move to the target location normally
    return 1;
}

static void piece_position(long int x, long int y)
{
    static int x1, y1, x2, y2;
    if (x < 300 || x > 970 || y < 25 || y > 780) {
        // Beyond the range of the chessboard, return directly
        return;
    }
    if (click_number == 0) {
        // First click, select chess piece
        x1 = (x - 320) / 75;
        y1 = (y - 35) / 75;
        chess = chess_board[y1][x1];
        if (chess == -1)
            return; // Selecting a blank chessboard will return
        if (turn_camp == redCamp) {
            if (piece[chess].camp != redCamp) {
                // Select your own faction chess piece and return directly
                return;
            } else {
                // Select non local faction chess pieces and switch factions
                turn_camp = blackCamp;
            }
        } else {
            if (piece[chess].camp != blackCamp) {
                // Select your own faction chess piece and return directly
                return;
            } else {
                // Select non local faction chess pieces and switch factions
                turn_camp = redCamp;
            }
        }
        changebuffer.last_index = chess;
        changebuffer.last_x = x1;
        changebuffer.last_y = y1;
        changebuffer.last_camp = piece[chess].camp;
        click_number++;
    } else if (click_number == 1) {
        // Do not reset click_numbers in this area, as the final judgment has not been completed here
        x2 = (x - 330) / 75;
        y2 = (y - 35) / 75;
        if (x1 == x2 && y1 == y2) {
            // If the positions are equal, do not execute. Choose again without changing other settings
            return;
        }
        chess2 = chess_board[y2][x2];
        changebuffer.new_x = x2;
        changebuffer.new_y = y2;
        changebuffer.new_index = chess2;
        changebuffer.new_camp = piece[chess2].camp;
        if (changebuffer.last_camp == changebuffer.new_camp) {
            x1 = x2;
            y1 = y2;
            chess = chess_board[y2][x2];
            changebuffer.last_index = chess;
            changebuffer.last_x = x1;
            changebuffer.last_y = y1;
            changebuffer.last_camp = piece[chess].camp;
            return;
        }
        piece_change(&changebuffer);
    }
}

static void piece_change(changestrcut* p)
{
    // Before changing the position of the chess piece, check if the movement of the chess piece is reasonable
    int result = piece[chess].judge(p->last_x, p->last_y, p->new_x, p->new_y, p->last_camp);
    if (result) {
        // Reasonable and normal execution of rules
        click_number = 0;
        // Change the position of chess pieces
        switch (p->last_index) {
        case 0:
            lv_obj_set_pos(Red_car_1, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 1:
            lv_obj_set_pos(Red_horse_1, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 2:
            lv_obj_set_pos(Red_elephant_1, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 3:
            lv_obj_set_pos(Red_shi_1, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 4:
            lv_obj_set_pos(Red_jiang, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 5:
            lv_obj_set_pos(Red_shi_2, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 6:
            lv_obj_set_pos(Red_elephant_2, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 7:
            lv_obj_set_pos(Red_horse_2, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 8:
            lv_obj_set_pos(Red_car_2, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 9:
            lv_obj_set_pos(Red_pao_1, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 10:
            lv_obj_set_pos(Red_pao_2, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 11:
            lv_obj_set_pos(Red_bing_1, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 12:
            lv_obj_set_pos(Red_bing_2, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 13:
            lv_obj_set_pos(Red_bing_3, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 14:
            lv_obj_set_pos(Red_bing_4, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 15:
            lv_obj_set_pos(Red_bing_5, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 16:
            lv_obj_set_pos(Black_car_1, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 17:
            lv_obj_set_pos(Black_horse_1, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 18:
            lv_obj_set_pos(Black_elephant_1, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 19:
            lv_obj_set_pos(Black_shi_1, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 20:
            lv_obj_set_pos(Black_jiang, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 21:
            lv_obj_set_pos(Black_shi_2, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 22:
            lv_obj_set_pos(Black_elephant_2, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 23:
            lv_obj_set_pos(Black_horse_2, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 24:
            lv_obj_set_pos(Black_car_2, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 25:
            lv_obj_set_pos(Black_pao_1, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 26:
            lv_obj_set_pos(Black_pao_2, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 27:
            lv_obj_set_pos(Black_bing_1, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 28:
            lv_obj_set_pos(Black_bing_2, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 29:
            lv_obj_set_pos(Black_bing_3, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 30:
            lv_obj_set_pos(Black_bing_4, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        case 31:
            lv_obj_set_pos(Black_bing_5, Board_Xvalue[p->new_x] - 35, Board_Yvalue[p->new_y] - 35);
            break;
        default:
            break;
        }
    } else {
        // Unreasonable rules return, generally unreasonable rules will not be executed to this step and will be excluded in the previous program
        return;
    }
    piece_information_change(p);
}

static int board_check(void)
{
    int i;
    int z = 0;
    if (piece_struct[4].Xvalue != piece_struct[20].Xvalue) {
        // Will not be in the same column as Shuai
        return 0;
    } else {
        // Will be in the same column as Shuai
        // Determine if there are any other chess pieces between Jiang and Shuai
        if (piece_struct[4].Yvalue < piece_struct[20].Yvalue) {
            for (i = piece_struct[4].Yvalue + 1; i < piece_struct[20].Yvalue; i++) {
                if (chess_board[i][piece_struct[4].Xvalue] != board_black) {
                    z++;
                    return 0;
                }
            }
        } else {
            for (i = piece_struct[20].Yvalue + 1; i < piece_struct[4].Yvalue; i++) {
                if (chess_board[i][piece_struct[4].Xvalue] != board_black) {
                    z++;
                    return 0;
                }
            }
        }
    }
    if (turn_camp == redCamp) {
        // Will be in the same column as Shuai, Red Chess wins
        return 2;
    } else {
        // Will be in the same column as Shuai, Black Chess wins
        return 1;
    }
}

static int board_check_shuai_statue(void)
{
    if (piece[4].statue == death) {
        // The red general is dead
        return 1;
    } else if (piece[20].statue == death) {
        // The black general is dead
        return 2;
    } else {
        // The game continues normally
        return 0;
    }
}

// Chessboard information update function
static void piece_information_change(changestrcut* p_information)
{
    int index_information;
    int chess_info;
    int chess_win_1, chess_win_2;
    // After the chess piece movement is completed, update the chessboard information, including the position and status of the chess piece.
    // Change chess piece information
    // If the target chess piece has an opponent chess piece, perform chess piece deletion operation
    if (chess_board[p_information->new_y][p_information->new_x] != board_black) {
        index_information = chess_board[p_information->new_y][p_information->new_x];
        piece[index_information].statue = death;
        // Delete chess piece image
        switch (index_information) {
        case 0:
            lv_obj_del(Red_car_1);
            Red_car_1 = NULL;
            break;
        case 1:
            lv_obj_del(Red_horse_1);
            Red_horse_1 = NULL;
            break;
        case 2:
            lv_obj_del(Red_elephant_1);
            Red_elephant_1 = NULL;
            break;
        case 3:
            lv_obj_del(Red_shi_1);
            Red_shi_1 = NULL;
            break;
        case 4:
            lv_obj_del(Red_jiang);
            Red_jiang = NULL;
            break;
        case 5:
            lv_obj_del(Red_shi_2);
            Red_shi_2 = NULL;
            break;
        case 6:
            lv_obj_del(Red_elephant_2);
            Red_elephant_2 = NULL;
            break;
        case 7:
            lv_obj_del(Red_horse_2);
            Red_horse_2 = NULL;
            break;
        case 8:
            lv_obj_del(Red_car_2);
            Red_car_2 = NULL;
            break;
        case 9:
            lv_obj_del(Red_pao_1);
            Red_pao_1 = NULL;
            break;
        case 10:
            lv_obj_del(Red_pao_2);
            Red_pao_2 = NULL;
            break;
        case 11:
            lv_obj_del(Red_bing_1);
            Red_bing_1 = NULL;
            break;
        case 12:
            lv_obj_del(Red_bing_2);
            Red_bing_2 = NULL;
            break;
        case 13:
            lv_obj_del(Red_bing_3);
            Red_bing_3 = NULL;
            break;
        case 14:
            lv_obj_del(Red_bing_4);
            Red_bing_4 = NULL;
            break;
        case 15:
            lv_obj_del(Red_bing_5);
            Red_bing_5 = NULL;
            break;
        case 16:
            lv_obj_del(Black_car_1);
            Black_car_1 = NULL;
            break;
        case 17:
            lv_obj_del(Black_horse_1);
            Black_horse_1 = NULL;
            break;
        case 18:
            lv_obj_del(Black_elephant_1);
            Black_elephant_1 = NULL;
            break;
        case 19:
            lv_obj_del(Black_shi_1);
            Black_shi_1 = NULL;
            break;
        case 20:
            lv_obj_del(Black_jiang);
            Black_jiang = NULL;
            break;
        case 21:
            lv_obj_del(Black_shi_2);
            Black_shi_2 = NULL;
            break;
        case 22:
            lv_obj_del(Black_elephant_2);
            Black_elephant_2 = NULL;
            break;
        case 23:
            lv_obj_del(Black_horse_2);
            Black_horse_2 = NULL;
            break;
        case 24:
            lv_obj_del(Black_car_2);
            Black_car_2 = NULL;
            break;
        case 25:
            lv_obj_del(Black_pao_1);
            Black_pao_1 = NULL;
            break;
        case 26:
            lv_obj_del(Black_pao_2);
            Black_pao_2 = NULL;
            break;
        case 27:
            lv_obj_del(Black_bing_1);
            Black_bing_1 = NULL;
            break;
        case 28:
            lv_obj_del(Black_bing_2);
            Black_bing_2 = NULL;
            break;
        case 29:
            lv_obj_del(Black_bing_3);
            Black_bing_3 = NULL;
            break;
        case 30:
            lv_obj_del(Black_bing_4);
            Black_bing_4 = NULL;
            break;
        case 31:
            lv_obj_del(Black_bing_5);
            Black_bing_5 = NULL;
            break;
        default:
            break;
        }
    }
    // Change the chessboard matrix
    chess_board[p_information->new_y][p_information->new_x] = chess_board[p_information->last_y][p_information->last_x];
    chess_board[p_information->last_y][p_information->last_x] = board_black;
    // Update chess piece position information
    chess_info = chess_board[p_information->new_y][p_information->new_x];
    piece_struct[chess_info].Xvalue = p_information->new_x;
    piece_struct[chess_info].Yvalue = p_information->new_y;
    chess_win_1 = board_check();
    chess_win_2 = board_check_shuai_statue();
    if (chess_win_1 != 0 || chess_win_2 != 0) {
        if (chess_win_2 != 0) {
            switch (chess_win_2) {
            case 1:
                // red gengeral, Black Chess wins
                reset_choose_page(chess_win_2);
                break;
            case 2:
                // balck gengeral, red Chess wins
                reset_choose_page(chess_win_2);
                break;
            default:
                break;
            }
        } else {
            switch (chess_win_1) {
            case 1:
                // General on the same straight line, Black Chess wins
                //  reset();
                reset_choose_page(chess_win_1);
                break;
            case 2:
                // reset();
                // The general is on the same straight line, and the red chess wins
                reset_choose_page(chess_win_1);
                break;
            default:
                break;
            }
        }
    }
}

static void reset(void)
{
    turn_camp = 1;
    lv_obj_t* pieces[] = {
        Red_car_1, Red_horse_1, Red_elephant_1, Red_shi_1, Red_jiang,
        Red_shi_2, Red_elephant_2, Red_horse_2, Red_car_2,
        Red_pao_1, Red_pao_2,
        Red_bing_1, Red_bing_2, Red_bing_3, Red_bing_4, Red_bing_5,

        Black_car_1, Black_horse_1, Black_elephant_1, Black_shi_1, Black_jiang,
        Black_shi_2, Black_elephant_2, Black_horse_2, Black_car_2,
        Black_pao_1, Black_pao_2,
        Black_bing_1, Black_bing_2, Black_bing_3, Black_bing_4, Black_bing_5
    };
    // Delete all chess piece objects
    int total_pieces = sizeof(pieces) / sizeof(pieces[0]);
    for (int k = 0; k < total_pieces; k++) {
        if (pieces[k]) {
            lv_obj_del(pieces[k]);
            pieces[k] = NULL;
        }
    }
    pieceStruct tmp_piece[32] = {
        { "redcar1", 0, 0, redCamp, car, alive },
        { "redhorse1", 1, 0, redCamp, horse, alive },
        { "redelephant1", 2, 0, redCamp, elephant, alive },
        { "redshi1", 3, 0, redCamp, shi, alive },
        { "redjiang", 4, 0, redCamp, jiang, alive },
        { "redshi2", 5, 0, redCamp, shi, alive },
        { "redelephant2", 6, 0, redCamp, elephant, alive },
        { "redhorse2", 7, 0, redCamp, horse, alive },
        { "redcar2", 8, 0, redCamp, car, alive },
        { "redpao1", 1, 2, redCamp, pao, alive },
        { "redpao2", 7, 2, redCamp, pao, alive },
        { "redbing1", 0, 3, redCamp, bing, alive },
        { "redbing2", 2, 3, redCamp, bing, alive },
        { "redbing3", 4, 3, redCamp, bing, alive },
        { "redbing4", 6, 3, redCamp, bing, alive },
        { "redbing5", 8, 3, redCamp, bing, alive },
        { "blackcar1", 0, 9, blackCamp, car, alive },
        { "blackhorse1", 1, 9, blackCamp, horse, alive },
        { "blackelephant1", 2, 9, blackCamp, elephant, alive },
        { "blackshi1", 3, 9, blackCamp, shi, alive },
        { "blackjiang", 4, 9, blackCamp, jiang, alive },
        { "blackshi2", 5, 9, blackCamp, shi, alive },
        { "blackelephant2", 6, 9, blackCamp, elephant, alive },
        { "blackhorse2", 7, 9, blackCamp, horse, alive },
        { "blackcar2", 8, 9, blackCamp, car, alive },
        { "blackpao1", 1, 7, blackCamp, pao, alive },
        { "blackpao2", 7, 7, blackCamp, pao, alive },
        { "blackzu1", 0, 6, blackCamp, zu, alive },
        { "blackzu2", 2, 6, blackCamp, zu, alive },
        { "blackzu3", 4, 6, blackCamp, zu, alive },
        { "blackzu4", 6, 6, blackCamp, zu, alive },
        { "blackzu5", 8, 6, blackCamp, zu, alive }
    };
    pieceStruct1 tmp_piece_struct[32] = {
        { "redcar1", 0, 0, redCamp },
        { "redhorse1", 1, 0, redCamp },
        { "redelephant1", 2, 0, redCamp },
        { "redshi1", 3, 0, redCamp },
        { "redjiang", 4, 0, redCamp },
        { "redshi2", 5, 0, redCamp },
        { "redelephant2", 6, 0, redCamp },
        { "redhorse2", 7, 0, redCamp },
        { "redcar2", 8, 0, redCamp },
        { "redpao1", 1, 2, redCamp },
        { "redpao2", 7, 2, redCamp },
        { "redbing1", 0, 3, redCamp },
        { "redbing2", 2, 3, redCamp },
        { "redbing3", 4, 3, redCamp },
        { "redbing4", 6, 3, redCamp },
        { "redbing5", 8, 3, redCamp },
        { "blackcar1", 0, 9, blackCamp },
        { "blackhorse1", 1, 9, blackCamp },
        { "blackelephant1", 2, 9, blackCamp },
        { "blackshi1", 3, 9, blackCamp },
        { "blackjiang", 4, 9, blackCamp },
        { "blackshi2", 5, 9, blackCamp },
        { "blackelephant2", 6, 9, blackCamp },
        { "blackhorse2", 7, 9, blackCamp },
        { "blackcar2", 8, 9, blackCamp },
        { "blackpao1", 1, 7, blackCamp },
        { "blackpao2", 7, 7, blackCamp },
        { "blackzu1", 0, 6, blackCamp },
        { "blackzu2", 2, 6, blackCamp },
        { "blackzu3", 4, 6, blackCamp },
        { "blackzu4", 6, 6, blackCamp },
        { "blackzu5", 8, 6, blackCamp }
    };
    changestrcut tmp_changebuffer = {
        0, 0, 0, 0, 0, 0, 0, 0
    };
    int tmp_chess_board[10][9] = {
        { red_che_1, red_ma_1, red_xiang_1, red_shi_1, red_jiang, red_shi_2, red_xiang_2, red_ma_2, red_che_2 },
        { board_black, board_black, board_black, board_black, board_black, board_black, board_black, board_black, board_black },
        { board_black, red_pao_1, board_black, board_black, board_black, board_black, board_black, red_pao_2, board_black },
        { red_bing_1, board_black, red_bing_2, board_black, red_bing_3, board_black, red_bing_4, board_black, red_bing_5 },
        { board_black, board_black, board_black, board_black, board_black, board_black, board_black, board_black, board_black },
        { board_black, board_black, board_black, board_black, board_black, board_black, board_black, board_black, board_black },
        { black_bing_1, board_black, black_bing_2, board_black, black_bing_3, board_black, black_bing_4, board_black, black_bing_5 },
        { board_black, black_pao_1, board_black, board_black, board_black, board_black, board_black, black_pao_2, board_black },
        { board_black, board_black, board_black, board_black, board_black, board_black, board_black, board_black, board_black },
        { black_che_1, black_ma_1, black_xiang_1, black_shi_1, black_jiang, black_shi_2, black_xiang_2, black_ma_2, black_che_2 }
    };
    // Copy to global variable
    memcpy(piece, tmp_piece, sizeof(tmp_piece));
    memcpy(piece_struct, tmp_piece_struct, sizeof(tmp_piece_struct));
    memcpy(chess_board, tmp_chess_board, sizeof(tmp_chess_board));
    changebuffer = tmp_changebuffer;
    position_init();
}

static void reset_choose_page(int choose_camp)
{
    lv_obj_t* root = lv_screen_active();
    lv_obj_set_style_bg_img_src(root, images.chessboard, LV_PART_MAIN);
    lv_obj_set_style_border_width(root, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(root, 0, LV_PART_MAIN);
    lv_obj_t* game_over_window = lv_obj_create(root);
    lv_obj_set_size(game_over_window, 200, 200);
    lv_obj_align(game_over_window, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(game_over_window, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_border_width(game_over_window, 2, LV_PART_MAIN);
    // Create a label within the window that displays' Game Over '
    lv_obj_t* label = lv_label_create(game_over_window);
    if (choose_camp == 1) {
        lv_label_set_text(label, "Black wins");
    } else if (choose_camp == 2) {
        lv_label_set_text(label, "Red wins");
    }
    // lv_label_set_text(label, "game over");
    // Adjust the attributes of the label (such as text color, alignment)
    lv_obj_set_style_text_color(label, lv_color_hex(0xFF0000), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0); // Center the text within the window
    // Create a 'restart' button
    lv_obj_t* btn_restart = lv_btn_create(game_over_window);
    lv_obj_set_size(btn_restart, 80, 40);
    lv_obj_align(btn_restart, LV_ALIGN_BOTTOM_LEFT, 10, -20);
    lv_obj_t* label_restart = lv_label_create(btn_restart);
    lv_label_set_text(label_restart, "Restart");
    lv_obj_center(label_restart);
    // Bind event callback to the 'restart' button
    lv_obj_add_event_cb(btn_restart, restart_event_cb, LV_EVENT_CLICKED, NULL);
    // Create an 'End' button
    lv_obj_t* btn_exit = lv_btn_create(game_over_window);
    lv_obj_set_size(btn_exit, 55, 40);
    lv_obj_align(btn_exit, LV_ALIGN_BOTTOM_RIGHT, -10, -20);

    lv_obj_t* label_exit = lv_label_create(btn_exit);
    lv_label_set_text(label_exit, "End");
    lv_obj_center(label_exit);
    lv_obj_add_event_cb(btn_exit, exit_event_cb, LV_EVENT_CLICKED, NULL);
}

static void app_create_error_page(void)
{
    lv_obj_t* root = lv_screen_active();
    lv_obj_t* label = lv_label_create(root);
    lv_obj_set_width(label, LV_PCT(80));
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(label, "Resource loading failed. \nPlease check the device and \nread the document for more details.");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_center(label);
}

static void restart_event_cb(lv_event_t* e)
{
    // Logic after clicking 'restart'
    lv_obj_t* btn = lv_event_get_target(e);
    lv_obj_t* win = lv_obj_get_parent(btn);
    lv_obj_del_async(win);
    reset();
}

static void exit_event_cb(lv_event_t* e)
{
    // Logic after clicking 'End'
    lv_obj_t* btn = lv_event_get_target(e);
    lv_obj_t* win = lv_obj_get_parent(btn);
    lv_obj_del_async(win);
}
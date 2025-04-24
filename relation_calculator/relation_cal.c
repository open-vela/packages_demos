#include "relation_cal.h"

static relation_cal_t g_rel_cal;


static const relation_transformation_t transitions[] = {
    // Self - related relationships
    {REL_SELF, REL_SELF, REL_SELF},  // Self to self remains self
    {REL_SELF, REL_FATHER, REL_FATHER},  // Self to father is father
    {REL_SELF, REL_MOTHER, REL_MOTHER},  // Self to mother is mother
    {REL_SELF, REL_SON, REL_SON},  // Self to son is son
    {REL_SELF, REL_DAUGHTER, REL_DAUGHTER},  // Self to daughter is daughter
    {REL_SELF, REL_ELDER_BROTHER, REL_ELDER_BROTHER},  // Self to elder brother is elder brother
    {REL_SELF, REL_YOUNGER_BROTHER, REL_YOUNGER_BROTHER},  // Self to younger brother is younger brother
    {REL_SELF, REL_ELDER_SISTER, REL_ELDER_SISTER},  // Self to elder sister is elder sister
    {REL_SELF, REL_YOUNGER_SISTER, REL_YOUNGER_SISTER},  // Self to younger sister is younger sister

    // Parent - child relationships
    {REL_FATHER, REL_SON, REL_SELF},  // Father to son is self
    {REL_FATHER, REL_DAUGHTER, REL_SELF},  // Father to daughter is self
    {REL_FATHER, REL_WIFE, REL_MOTHER},
    {REL_MOTHER, REL_SON, REL_SELF},  // Mother to son is self
    {REL_MOTHER, REL_DAUGHTER, REL_SELF},  // Mother to daughter is self
    {REL_MOTHER, REL_HUSBAND, REL_FATHER},
    {REL_SON, REL_FATHER, REL_FATHER},  // Son to father is father
    {REL_SON, REL_MOTHER, REL_MOTHER},  // Son to mother is mother
    {REL_DAUGHTER, REL_FATHER, REL_FATHER},  // Daughter to father is father
    {REL_DAUGHTER, REL_MOTHER, REL_MOTHER},  // Daughter to mother is mother

    // Grandparent relationships
    {REL_FATHER, REL_FATHER, REL_GRANDFATHER},  // Father's father is grandfather
    {REL_FATHER, REL_MOTHER, REL_GRANDMOTHER},  // Father's mother is grandmother
    {REL_MOTHER, REL_FATHER, REL_GRANDFATHER},  // Mother's father is grandfather
    {REL_MOTHER, REL_MOTHER, REL_GRANDMOTHER},  // Mother's mother is grandmother
    {REL_GRANDFATHER, REL_SON, REL_UNCLE},  // Grandfather to son is REL_UNCLE
    {REL_GRANDFATHER, REL_DAUGHTER, REL_AUNT},  // Grandfather to daughter is aunt
    {REL_GRANDMOTHER, REL_SON, REL_FATHER},  // Grandmother to son is father
    {REL_GRANDMOTHER, REL_DAUGHTER, REL_AUNT},  // Grandmother to daughter is aunt

    // Great - grandparent relationships
    {REL_GRANDFATHER, REL_FATHER, REL_GREAT_GRANDFATHER},  // Grandfather's father is great - grandfather
    {REL_GRANDFATHER, REL_MOTHER, REL_GREAT_GRANDMOTHER},  // Grandfather's mother is great - grandmother
    {REL_GRANDMOTHER, REL_FATHER, REL_GREAT_GRANDFATHER},  // Grandmother's father is great - grandfather
    {REL_GRANDMOTHER, REL_MOTHER, REL_GREAT_GRANDMOTHER},  // Grandmother's mother is great - grandmother
    {REL_MATERNAL_GRANDFATHER, REL_FATHER, REL_MATERNAL_GREAT_GRANDFATHER},  // Maternal grandfather's father is maternal great - grandfather
    {REL_MATERNAL_GRANDFATHER, REL_MOTHER, REL_MATERNAL_GREAT_GRANDMOTHER},  // Maternal grandfather's mother is maternal great - grandmother
    {REL_MATERNAL_GRANDMOTHER, REL_FATHER, REL_MATERNAL_GREAT_GRANDFATHER},  // Maternal grandmother's father is maternal great - grandfather
    {REL_MATERNAL_GRANDMOTHER, REL_MOTHER, REL_MATERNAL_GREAT_GRANDMOTHER},  // Maternal grandmother's mother is maternal great - grandmother

    // Sibling relationships
    {REL_ELDER_BROTHER, REL_FATHER, REL_ELDER_BROTHER},  // Elder brother to father remains elder brother
    {REL_ELDER_BROTHER, REL_MOTHER, REL_ELDER_BROTHER},  // Elder brother to mother remains elder brother
    {REL_YOUNGER_BROTHER, REL_FATHER, REL_YOUNGER_BROTHER},  // Younger brother to father remains younger brother
    {REL_YOUNGER_BROTHER, REL_MOTHER, REL_YOUNGER_BROTHER},  // Younger brother to mother remains younger brother
    {REL_ELDER_SISTER, REL_FATHER, REL_ELDER_SISTER},  // Elder sister to father remains elder sister
    {REL_ELDER_SISTER, REL_MOTHER, REL_ELDER_SISTER},  // Elder sister to mother remains elder sister
    {REL_YOUNGER_SISTER, REL_FATHER, REL_YOUNGER_SISTER},  // Younger sister to father remains younger sister
    {REL_YOUNGER_SISTER, REL_MOTHER, REL_YOUNGER_SISTER},  // Younger sister to mother remains younger sister
    {REL_ELDER_BROTHER, REL_ELDER_BROTHER, REL_ELDER_BROTHER},  // Elder brother to elder brother remains elder brother
    {REL_ELDER_BROTHER, REL_YOUNGER_BROTHER, REL_YOUNGER_BROTHER},  // Elder brother to younger brother remains elder brother
    {REL_YOUNGER_BROTHER, REL_ELDER_BROTHER, REL_ELDER_BROTHER},  // Younger brother to elder brother remains younger brother
    {REL_YOUNGER_BROTHER, REL_YOUNGER_BROTHER, REL_YOUNGER_BROTHER},  // Younger brother to younger brother remains younger brother
    {REL_ELDER_SISTER, REL_ELDER_SISTER, REL_ELDER_SISTER},  // Elder sister to elder sister remains elder sister
    {REL_ELDER_SISTER, REL_YOUNGER_SISTER, REL_ELDER_SISTER},  // Elder sister to younger sister remains elder sister
    {REL_YOUNGER_SISTER, REL_ELDER_SISTER, REL_YOUNGER_SISTER},  // Younger sister to elder sister remains younger sister
    {REL_YOUNGER_SISTER, REL_YOUNGER_SISTER, REL_YOUNGER_SISTER},  // Younger sister to younger sister remains younger sister

    // Spouse relationships
    {REL_WIFE, REL_HUSBAND, REL_WIFE},  // Wife to husband remains wife
    {REL_HUSBAND, REL_WIFE, REL_HUSBAND},  // Husband to wife remains husband

    // Uncle and aunt relationships
    {REL_FATHER, REL_ELDER_BROTHER, REL_UNCLE},  // Father's elder brother is uncle
    {REL_FATHER, REL_YOUNGER_BROTHER, REL_UNCLE},  // Father's younger brother is uncle
    {REL_FATHER, REL_ELDER_SISTER, REL_AUNT},  // Father's elder sister is aunt
    {REL_FATHER, REL_YOUNGER_SISTER, REL_AUNT},  // Father's younger sister is aunt
    {REL_MOTHER, REL_ELDER_BROTHER, REL_MATERNAL_UNCLE},  // Mother's elder brother is maternal uncle
    {REL_MOTHER, REL_YOUNGER_BROTHER, REL_MATERNAL_UNCLE},  // Mother's younger brother is maternal uncle
    {REL_MOTHER, REL_ELDER_SISTER, REL_MATERNAL_AUNT},  // Mother's elder sister is maternal aunt
    {REL_MOTHER, REL_YOUNGER_SISTER, REL_MATERNAL_AUNT},  // Mother's younger sister is maternal aunt
    {REL_UNCLE, REL_SON, REL_COUSIN_MALE},  // Uncle to son is male cousin
    {REL_UNCLE, REL_DAUGHTER, REL_COUSIN_FEMALE},  // Uncle to daughter is female cousin
    {REL_AUNT, REL_SON, REL_COUSIN_MALE},  // Aunt to son is male cousin
    {REL_AUNT, REL_DAUGHTER, REL_COUSIN_FEMALE},  // Aunt to daughter is female cousin
    {REL_MATERNAL_UNCLE, REL_SON, REL_COUSIN_MALE},  // Maternal uncle to son is male cousin
    {REL_MATERNAL_UNCLE, REL_DAUGHTER, REL_COUSIN_FEMALE},  // Maternal uncle to daughter is female cousin
    {REL_MATERNAL_AUNT, REL_SON, REL_COUSIN_MALE},  // Maternal aunt to son is male cousin
    {REL_MATERNAL_AUNT, REL_DAUGHTER, REL_COUSIN_FEMALE},  // Maternal aunt to daughter is female cousin

    // Cousin relationships
    {REL_COUSIN_MALE, REL_FATHER, REL_COUSIN_MALE},  // Male cousin to father remains male cousin
    {REL_COUSIN_MALE, REL_MOTHER, REL_COUSIN_MALE},  // Male cousin to mother remains male cousin
    {REL_COUSIN_FEMALE, REL_FATHER, REL_COUSIN_FEMALE},  // Female cousin to father remains female cousin
    {REL_COUSIN_FEMALE, REL_MOTHER, REL_COUSIN_FEMALE},  // Female cousin to mother remains female cousin

    // Great - uncle and great - aunt relationships
    {REL_GRANDFATHER, REL_ELDER_BROTHER, REL_GREAT_UNCLE},  // Grandfather's elder brother is great - uncle
    {REL_GRANDFATHER, REL_YOUNGER_BROTHER, REL_GREAT_UNCLE},  // Grandfather's younger brother is great - uncle
    {REL_GRANDFATHER, REL_ELDER_SISTER, REL_GREAT_AUNT},  // Grandfather's elder sister is great - aunt
    {REL_GRANDFATHER, REL_YOUNGER_SISTER, REL_GREAT_AUNT},  // Grandfather's younger sister is great - aunt
    {REL_GREAT_UNCLE, REL_SON, REL_FIRST_COUSIN_ONCE_REMOVED_MALE},  // Great - uncle to son is male first cousin once removed
    {REL_GREAT_UNCLE, REL_DAUGHTER, REL_FIRST_COUSIN_ONCE_REMOVED_FEMALE},  // Great - uncle to daughter is female first cousin once removed
    {REL_GREAT_AUNT, REL_SON, REL_FIRST_COUSIN_ONCE_REMOVED_MALE},  // Great - aunt to son is male first cousin once removed
    {REL_GREAT_AUNT, REL_DAUGHTER, REL_FIRST_COUSIN_ONCE_REMOVED_FEMALE},  // Great - aunt to daughter is female first cousin once removed

    // Second - cousin relationships
    {REL_FIRST_COUSIN_ONCE_REMOVED_MALE, REL_SON, REL_SECOND_COUSIN_MALE},  // Male first cousin once removed to son is male second cousin
    {REL_FIRST_COUSIN_ONCE_REMOVED_MALE, REL_DAUGHTER, REL_SECOND_COUSIN_FEMALE},  // Male first cousin once removed to daughter is female second cousin
    {REL_FIRST_COUSIN_ONCE_REMOVED_FEMALE, REL_SON, REL_SECOND_COUSIN_MALE},  // Female first cousin once removed to son is male second cousin
    {REL_FIRST_COUSIN_ONCE_REMOVED_FEMALE, REL_DAUGHTER, REL_SECOND_COUSIN_FEMALE},  // Female first cousin once removed to daughter is female second cousin

    // Nephew and niece relationships
    {REL_ELDER_BROTHER, REL_SON, REL_NEPHEW},  // Elder brother to son is nephew
    {REL_ELDER_BROTHER, REL_DAUGHTER, REL_NIECE},  // Elder brother to daughter is niece
    {REL_YOUNGER_BROTHER, REL_SON, REL_NEPHEW},  // Younger brother to son is nephew
    {REL_YOUNGER_BROTHER, REL_DAUGHTER, REL_NIECE},  // Younger brother to daughter is niece
    {REL_ELDER_SISTER, REL_SON, REL_NEPHEW},  // Elder sister to son is nephew
    {REL_ELDER_SISTER, REL_DAUGHTER, REL_NIECE},  // Elder sister to daughter is niece
    {REL_YOUNGER_SISTER, REL_SON, REL_NEPHEW},  // Younger sister to son is nephew
    {REL_YOUNGER_SISTER, REL_DAUGHTER, REL_NIECE},  // Younger sister to daughter is niece

    {REL_SELF, REL_SON_IN_LAW, REL_SON_IN_LAW},  // 自己到儿媳是儿媳
    {REL_SELF, REL_DAUGHTER_IN_LAW, REL_DAUGHTER_IN_LAW},  // 自己到女婿是女婿
    {REL_SELF, REL_GRANDSON, REL_GRANDSON},  // 自己到孙子是孙子
    {REL_SELF, REL_GRANDDAUGHTER, REL_GRANDDAUGHTER},  // 自己到孙女是孙女
    {REL_SELF, REL_GREAT_GRANDSON, REL_GREAT_GRANDSON},  // 自己到曾孙子是曾孙子
    {REL_SELF, REL_GREAT_GRANDDAUGHTER, REL_GREAT_GRANDDAUGHTER},  // 自己到曾孙女是曾孙女
    
    {REL_SON, REL_SON, REL_GRANDSON},  // 儿子到儿子是孙子
    {REL_SON, REL_DAUGHTER, REL_GRANDDAUGHTER},  // 儿子到女儿是孙女
    {REL_DAUGHTER, REL_SON, REL_GRANDSON},  // 女儿到儿子是孙子
    {REL_DAUGHTER, REL_DAUGHTER, REL_GRANDDAUGHTER},  // 女儿到女儿是孙女
    
    {REL_GRANDSON, REL_FATHER, REL_SON},  // 孙子到父亲是儿子
    {REL_GRANDSON, REL_MOTHER, REL_DAUGHTER},  // 孙子到母亲是女儿
    {REL_GRANDDAUGHTER, REL_FATHER, REL_SON},  // 孙女到父亲是儿子
    {REL_GRANDDAUGHTER, REL_MOTHER, REL_DAUGHTER},  // 孙女到母亲是女儿
    
    {REL_SON, REL_SON_IN_LAW, REL_DAUGHTER_IN_LAW},  // 儿子到儿媳是儿媳（需调整）
    {REL_DAUGHTER, REL_DAUGHTER_IN_LAW, REL_SON_IN_LAW},  // 女儿到女婿是女婿（需调整）
    
    {REL_SON_IN_LAW, REL_HUSBAND, REL_SON},  // 儿媳到丈夫是儿子
    {REL_DAUGHTER_IN_LAW, REL_WIFE, REL_DAUGHTER},  // 女婿到妻子是女儿
    
    {REL_GRANDSON, REL_SON, REL_GREAT_GRANDSON},  // 孙子到儿子是曾孙子
    {REL_GRANDDAUGHTER, REL_DAUGHTER, REL_GREAT_GRANDDAUGHTER},  // 孙女到女儿是曾孙女
    
    // 新增的其他子辈关系
    {REL_SON, REL_SON, REL_GRANDSON},  // 儿子到儿子是孙子
    {REL_SON, REL_DAUGHTER, REL_GRANDDAUGHTER},  // 儿子到女儿是孙女
    {REL_DAUGHTER, REL_SON, REL_GRANDSON},  // 女儿到儿子是孙子
    {REL_DAUGHTER, REL_DAUGHTER, REL_GRANDDAUGHTER},  // 女儿到女儿是孙女
    
    {REL_NEPHEW, REL_SON, REL_GRAND_NEPHEW},  // 侄子到儿子是孙侄
    {REL_NEPHEW, REL_DAUGHTER, REL_GRAND_NIECE},  // 侄子到女儿是孙侄女
    {REL_NIECE, REL_SON, REL_GRAND_NEPHEW},  // 侄女到儿子是孙侄
    {REL_NIECE, REL_DAUGHTER, REL_GRAND_NIECE},  // 侄女到女儿是孙侄女
    
    {REL_COUSIN_MALE, REL_SON, REL_COUSIN_SON},  // 堂兄弟到儿子是堂侄
    {REL_COUSIN_MALE, REL_DAUGHTER, REL_COUSIN_DAUGHTER},  // 堂兄弟到女儿是堂侄女
    {REL_COUSIN_FEMALE, REL_SON, REL_COUSIN_SON},  // 堂姐妹到儿子是堂侄
    {REL_COUSIN_FEMALE, REL_DAUGHTER, REL_COUSIN_DAUGHTER},  // 堂姐妹到女儿是堂侄女
    
    {REL_SON, REL_WIFE, REL_SON_IN_LAW},  // 儿子到妻子是儿媳
    {REL_DAUGHTER, REL_HUSBAND, REL_DAUGHTER_IN_LAW},  // 女儿到丈夫是女婿
    
    {REL_SON_IN_LAW, REL_FATHER, REL_SON},  // 儿媳到父亲是儿子
    {REL_DAUGHTER_IN_LAW, REL_FATHER, REL_DAUGHTER},  // 女婿到父亲是女儿
    
    {REL_NULL, REL_NULL, REL_NULL}  // End marker
};

static const char *relation_names[] = {
    "自己",          // REL_SELF
    "父亲",          // REL_FATHER
    "母亲",          // REL_MOTHER
    "儿子",          // REL_SON
    "女儿",          // REL_DAUGHTER
    "妻子",          // REL_WIFE
    "丈夫",          // REL_HUSBAND
    "哥哥",          // REL_ELDER_BROTHER
    "弟弟",          // REL_YOUNGER_BROTHER
    "姐姐",          // REL_ELDER_SISTER
    "妹妹",          // REL_YOUNGER_SISTER
    "祖父",          // REL_GRANDFATHER
    "祖母",          // REL_GRANDMOTHER
    "伯父",          // REL_UNCLE
    "姑母",          // REL_AUNT
    "堂兄弟",        // REL_COUSIN_MALE
    "堂姐妹",        // REL_COUSIN_FEMALE
    "曾祖父",        // REL_GREAT_GRANDFATHER
    "曾祖母",        // REL_GREAT_GRANDMOTHER
    "外祖父",        // REL_MATERNAL_GRANDFATHER
    "外祖母",        // REL_MATERNAL_GRANDMOTHER
    "外曾祖父",      // REL_MATERNAL_GREAT_GRANDFATHER
    "外曾祖母",      // REL_MATERNAL_GREAT_GRANDMOTHER
    "表侄",          // REL_FIRST_COUSIN_ONCE_REMOVED_MALE
    "表侄女",        // REL_FIRST_COUSIN_ONCE_REMOVED_FEMALE
    "再从兄弟",      // REL_SECOND_COUSIN_MALE
    "再从姐妹",      // REL_SECOND_COUSIN_FEMALE
    "叔祖父",        // REL_GREAT_UNCLE
    "姑祖母",        // REL_GREAT_AUNT
    "舅父",          // REL_MATERNAL_UNCLE
    "姨母",          // REL_MATERNAL_AUNT
    "侄子",          // REL_NEPHEW
    "侄女",          // REL_NIECE
    "儿媳",                   // REL_SON_IN_LAW
    "女婿",                   // REL_DAUGHTER_IN_LAW
    "孙子",                  // REL_GRANDSON
    "孙女",                  // REL_GRANDDAUGHTER
    "曾孙子",                // REL_GREAT_GRANDSON
    "曾孙女",                // REL_GREAT_GRANDDAUGHTER
    "堂侄/表侄",             // REL_COUSIN_SON
    "堂侄女/表侄女",         // REL_COUSIN_DAUGHTER
    "孙侄",                  // REL_GRAND_NEPHEW
    "孙侄女",                // REL_GRAND_NIECE
    "未知"                   // REL_NULL
};

static const btnm_relation_t btnm_relation[] = {
    {"父亲", REL_FATHER, GENDER_MALE},
    {"母亲", REL_MOTHER, GENDER_FEMALE},
    {"丈夫", REL_HUSBAND, GENDER_MALE},
    {"妻子", REL_WIFE, GENDER_FEMALE},
    {"儿子", REL_SON, GENDER_MALE},
    {"女儿", REL_DAUGHTER, GENDER_FEMALE},
    {"哥哥", REL_ELDER_BROTHER, GENDER_MALE},
    {"弟弟", REL_YOUNGER_BROTHER, GENDER_MALE},
    {"姐姐", REL_ELDER_SISTER, GENDER_FEMALE},
    {"妹妹", REL_YOUNGER_SISTER, GENDER_FEMALE}
};

static const char *btnm_map[] = {
    "父亲", "丈夫", "儿子", "哥哥", "姐姐", "<-", "清除", "\n",
    "母亲", "妻子", "女儿", "弟弟", "妹妹", " " , "计算", ""  };

static void rel_cal_main_page(void);
static void rel_cal_btnmatrix_event_cb(lv_event_t *e);
static void rel_transform_handle(const char *text);
static const char *calculate_relationship(relation_cal_t *self);

void relation_cal_app_create(void) 
{
    g_rel_cal.rel_count = 0;
    rel_cal_main_page();    
}

static void rel_cal_main_page(void)
{
    LV_FONT_DECLARE(lv_font_chinese_siyuan_16);

    // 创建主容器
    lv_obj_t *root = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(root);
    lv_obj_center(root);
    lv_obj_set_style_bg_color(root, lv_color_white(), 0);
    lv_obj_set_style_shadow_color(root, lv_color_hex(0xE4E7EB), 0);
    lv_obj_set_style_shadow_width(root, 40, 0);
    lv_obj_align(root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(root, 760, 460);
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(root, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);

    // 创建标题
    g_rel_cal.ui.title = lv_label_create(root);
    lv_obj_set_size(g_rel_cal.ui.title, 760, 50);
    lv_label_set_text(g_rel_cal.ui.title, "亲属关系计算器");
    lv_obj_set_style_text_font(g_rel_cal.ui.title, &lv_font_chinese_siyuan_16, 0);
    lv_obj_set_style_text_color(g_rel_cal.ui.title, lv_color_hex(0x2C3E50), 0);
    lv_obj_set_style_text_align(g_rel_cal.ui.title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_pad_top(g_rel_cal.ui.title, 10, 0);
    // 修改标题颜色为橙色
    lv_obj_set_style_text_color(g_rel_cal.ui.title, lv_color_hex(0xFFA500), 0);

    // 创建显示区域
    g_rel_cal.ui.screen = lv_textarea_create(root);
    lv_obj_set_size(g_rel_cal.ui.screen, LV_PCT(100), LV_PCT(40));
    lv_obj_set_style_text_font(g_rel_cal.ui.screen, &lv_font_chinese_siyuan_16, 0);
    lv_obj_set_style_bg_color(g_rel_cal.ui.screen, lv_color_hex(0x2C3E50), 0);
    lv_obj_set_style_text_color(g_rel_cal.ui.screen, lv_color_white(), 0);
    lv_obj_set_style_radius(g_rel_cal.ui.screen, 20, 0);
    lv_obj_set_style_pad_all(g_rel_cal.ui.screen, 15, 0);
    lv_textarea_set_cursor_click_pos(g_rel_cal.ui.screen, false);
    lv_textarea_set_max_length(g_rel_cal.ui.screen, 128);
    lv_textarea_set_align(g_rel_cal.ui.screen, LV_TEXT_ALIGN_LEFT);
    lv_textarea_set_text(g_rel_cal.ui.screen, "");

    // 创建按钮矩阵
    g_rel_cal.ui.btnm = lv_btnmatrix_create(root);
    lv_obj_set_style_border_width(g_rel_cal.ui.btnm, 0, 0);
    lv_obj_set_size(g_rel_cal.ui.btnm, LV_PCT(100), LV_PCT(40));
    lv_obj_set_style_text_font(g_rel_cal.ui.btnm, &lv_font_chinese_siyuan_16, LV_PART_MAIN);
    lv_buttonmatrix_set_map(g_rel_cal.ui.btnm, btnm_map);
    lv_obj_add_event_cb(g_rel_cal.ui.btnm, rel_cal_btnmatrix_event_cb, LV_EVENT_VALUE_CHANGED, root);
    lv_obj_set_style_bg_color(g_rel_cal.ui.btnm, lv_color_hex(0xECF0F1), 0);
    lv_obj_set_style_radius(g_rel_cal.ui.btnm, 10, 0);
    lv_obj_set_style_pad_all(g_rel_cal.ui.btnm, 10, 0);
    

    // 创建提示信息
    g_rel_cal.ui.note = lv_label_create(root);
    lv_obj_set_size(g_rel_cal.ui.note, LV_PCT(100), LV_PCT(10));
    lv_label_set_text(g_rel_cal.ui.note, "这是一个亲属关系计算器,这只是v1的简陋版。");
    lv_obj_set_style_text_font(g_rel_cal.ui.note, &lv_font_chinese_siyuan_16, 0);
    lv_obj_set_style_text_color(g_rel_cal.ui.note, lv_color_hex(0x7F8C8D), 0);
    lv_obj_set_style_text_align(g_rel_cal.ui.note, LV_TEXT_ALIGN_CENTER, 0);
}


static void rel_cal_btnmatrix_event_cb(lv_event_t *e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    uint8_t id = lv_buttonmatrix_get_selected_button(obj);
    const char * text = lv_buttonmatrix_get_button_text(obj, id);
    rel_transform_handle(text);
}

static void rel_transform_handle(const char *text)
{
    static btnm_relation_t last_input = {"?", REL_NULL,GENDER_UNKNOWN};

    if(g_rel_cal.status == STATUS_CALCULATED)
    {
        g_rel_cal.status = STATUS_INPUTTING;
        lv_textarea_set_text(g_rel_cal.ui.screen,"");
    }

    if (0 == strcmp(text,"计算")) 
    {
        if(g_rel_cal.rel_count > 0)
        {
            const char *result = calculate_relationship(&g_rel_cal);
            lv_textarea_set_text(g_rel_cal.ui.screen,result); 
            g_rel_cal.rel_count = 0;
        }else
        {
            lv_textarea_set_text(g_rel_cal.ui.screen,"自己");
            g_rel_cal.rel_count = 0;
            
        }
        g_rel_cal.status = STATUS_CALCULATED;
        last_input.relation = REL_NULL;
        last_input.gender = GENDER_UNKNOWN;
    }
    else if(0 == strcmp(text,"清除")) 
    {
        lv_textarea_set_text(g_rel_cal.ui.screen,"");
        g_rel_cal.rel_count = 0;
        
        last_input.relation = REL_NULL;
        last_input.gender = GENDER_UNKNOWN;
    }
    else if(0 == strcmp(text,"<-")) 
    {
        if(g_rel_cal.rel_count)
        {
            g_rel_cal.rel_count--;
            //TBD:delete
            lv_textarea_delete_char(g_rel_cal.ui.screen);
            lv_textarea_delete_char(g_rel_cal.ui.screen);
            lv_textarea_delete_char(g_rel_cal.ui.screen);
        }
    }
    else if(0 == strcmp(text," "))
    {
        return;
    }
    else 
    {
        for(int i=0; i <  sizeof(btnm_relation) / sizeof(btnm_relation[0]);i++)
        {
            if( 0 == strcmp(text,btnm_relation[i].btnm_text))
            {
                if( (GENDER_FEMALE  == last_input.gender                         &&\
                        REL_WIFE       == btnm_relation[i].relation)             ||\
                    (GENDER_MALE == last_input.gender                            &&\
                        REL_HUSBAND == btnm_relation[i].relation)         
                    )
                {
                    return;
                }
                if(g_rel_cal.rel_count < MAX_REL_LEN )
                {
                    g_rel_cal.rel_list[g_rel_cal.rel_count++] = btnm_relation[i].relation;
                }
                
                last_input = btnm_relation[i];
                break;
            }
        }
        lv_textarea_add_text(g_rel_cal.ui.screen, text);
        lv_textarea_add_text(g_rel_cal.ui.screen, "的");
    }
}


static const char *calculate_relationship(relation_cal_t *self) {
    relation_type_t current = REL_SELF;
    for (int i = 0; i < self->rel_count; i++) {
        int found = 0;
        for (int j = 0; transitions[j].from != REL_NULL; j++) {
            if (transitions[j].from == current && transitions[j].to == self->rel_list[i]) {
                current = transitions[j].result;
                found = 1;
                break;
            }
        }
        if (!found) {
            current = REL_NULL;
            break;
        }
    }

    return relation_names[current];
}

#include "relation_cal.h"

static relation_tree_t g_relation_tree[MAX_TREE_LEN];

static relation_cal_t g_rel_cal;

static const btnm_relation_t btnm_relation[] = {
    {"父亲", REL_FATHER,GENDER_MALE}          , {"母亲", REL_MOTHER,GENDER_FEMALE} ,
    {"丈夫", REL_HUSBAND,GENDER_MALE}         , {"妻子", REL_WIFE,GENDER_FEMALE} ,
    {"儿子", REL_SON,GENDER_MALE}             , {"女儿", REL_DAUGHTER,GENDER_FEMALE} ,
    {"哥哥", REL_ELDER_BROTHER,GENDER_MALE}   , {"弟弟", REL_YOUNGER_BROTHER,GENDER_MALE} ,
    {"姐姐", REL_ELDER_SISTER,GENDER_FEMALE}  , {"妹妹", REL_YOUNGER_SISTER,GENDER_FEMALE} 
};


static const char *btnm_map[] = {
    "父亲", "丈夫", "儿子", "哥哥", "姐姐", "<-", "清除", "\n",
    "母亲", "妻子", "女儿", "弟弟", "妹妹", " " , "计算", ""  };


static uint8_t init_relation_tree(relation_tree_t *self,relation_tree_t *relaion_buffer);
static uint8_t add_relation_node(relation_type_t relation,relation_cal_t *self);
static uint8_t delete_relation_node(relation_cal_t *self);
static uint8_t clear_relation_node(relation_cal_t *self);
static const char *calculate_relationship(relation_cal_t *self);
static const char  *navigate_relationship(relation_tree_t *current,relation_node_t *rel);
static void rel_cal_main_page(void);
static void rel_cal_btnmatrix_event_cb(lv_event_t *e);

void relation_cal_app_create(void)
{
    init_relation_tree(&g_rel_cal.mine,g_relation_tree);
    rel_cal_main_page();
}


static void rel_cal_main_page(void)
{
    LV_FONT_DECLARE(lv_font_chinese_siyuan_16);

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

    g_rel_cal.ui.screen = lv_textarea_create(root);
    lv_obj_set_size(g_rel_cal.ui.screen, LV_PCT(100), LV_PCT(40));    
    lv_obj_set_style_text_font(g_rel_cal.ui.screen, &lv_font_chinese_siyuan_16, 0);
    lv_obj_set_style_bg_color(g_rel_cal.ui.screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_width(root, 40, 0); 
    lv_obj_set_style_radius(g_rel_cal.ui.screen, 20, 0);
    lv_obj_set_style_pad_all(g_rel_cal.ui.screen, 15, 0);

    lv_obj_set_style_bg_grad_color(g_rel_cal.ui.screen, lv_color_hex(0x2C3E50), 0); 

    lv_obj_set_style_bg_main_stop(g_rel_cal.ui.screen, 200, 0);
    lv_obj_set_style_text_color(g_rel_cal.ui.screen, lv_color_white(), 0); 

    lv_textarea_set_cursor_click_pos(g_rel_cal.ui.screen, false);
    lv_textarea_set_max_length(g_rel_cal.ui.screen, 128);
    lv_textarea_set_align(g_rel_cal.ui.screen, LV_TEXT_ALIGN_LEFT);
    lv_textarea_set_text(g_rel_cal.ui.screen, "");

    g_rel_cal.ui.btnm = lv_btnmatrix_create(root);
    lv_obj_set_style_border_width(g_rel_cal.ui.btnm, 0, 0);
    lv_obj_set_size(g_rel_cal.ui.btnm, LV_PCT(100), LV_PCT(40));
    lv_obj_set_style_text_font(g_rel_cal.ui.btnm,&lv_font_chinese_siyuan_16,LV_PART_MAIN);
    lv_buttonmatrix_set_map(g_rel_cal.ui.btnm, btnm_map);
    lv_obj_add_event_cb(g_rel_cal.ui.btnm,rel_cal_btnmatrix_event_cb,LV_EVENT_VALUE_CHANGED,root);


    g_rel_cal.ui.note = lv_label_create(root);
    lv_obj_set_size(g_rel_cal.ui.note, LV_PCT(100), LV_PCT(10));
    lv_label_set_text(g_rel_cal.ui.note, "这是一个亲属关系计算器,这只是v0的简陋版。");
    lv_obj_set_style_text_font(g_rel_cal.ui.note, &lv_font_chinese_siyuan_16, 0);
    lv_obj_set_style_text_color(g_rel_cal.ui.note, lv_color_hex(0x7F8C8D), 0); 
    
    g_rel_cal.ui.title = lv_label_create(root);
    lv_obj_set_size(g_rel_cal.ui.title, 760, 50);
    lv_label_set_text(g_rel_cal.ui.title, "亲属关系计算器");
    int fontHeight = lv_font_get_line_height(LV_FONT_DEFAULT);
    int verPad = (50 - fontHeight)/2;
    lv_obj_set_style_pad_top(g_rel_cal.ui.title, verPad, 0);
    int textWidth = lv_txt_get_width("亲属关系计算器", strlen("亲属关系计算器"), LV_FONT_DEFAULT, 0);
    int horPad = (760 - textWidth)/2;
    lv_obj_set_style_pad_left(g_rel_cal.ui.title, horPad, 0);
    lv_obj_set_style_text_font(g_rel_cal.ui.title, &lv_font_chinese_siyuan_16, 0);
    lv_obj_set_style_text_color(g_rel_cal.ui.title, lv_color_hex(0x2C3E50), 0);
}


static void rel_cal_btnmatrix_event_cb(lv_event_t *e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    uint8_t id = lv_buttonmatrix_get_selected_button(obj);
    const char * text = lv_buttonmatrix_get_button_text(obj, id);
    static btnm_relation_t last_input = {"?", REL_NULL,GENDER_UNKNOWN};
    if (0 == strcmp(text,"计算")) 
    {
        if(g_rel_cal.rel_list != NULL)
        {
            const char *result = calculate_relationship(&g_rel_cal);
            lv_textarea_set_text(g_rel_cal.ui.screen,result); 
            g_rel_cal.status = REL_NODE_SUCCESS;
            clear_relation_node(&g_rel_cal);
        }else
        {
            lv_textarea_set_text(g_rel_cal.ui.screen,"自己"); 
            g_rel_cal.status = REL_NODE_SUCCESS;            
        }
        last_input.relation = REL_NULL;
        last_input.gender = GENDER_UNKNOWN;

    }
    else if(0 == strcmp(text,"清除")) 
    {
        lv_textarea_set_text(g_rel_cal.ui.screen,"");
        clear_relation_node(&g_rel_cal);
        last_input.relation = REL_NULL;
        last_input.gender = GENDER_UNKNOWN;
    }
    else if(0 == strcmp(text,"<-")) 
    {
        if(g_rel_cal.rel_list)
        {
            delete_relation_node(&g_rel_cal);
            lv_textarea_delete_char(g_rel_cal.ui.screen);
            lv_textarea_delete_char(g_rel_cal.ui.screen);
            lv_textarea_delete_char(g_rel_cal.ui.screen);

            last_input.relation = REL_NULL;
            last_input.gender = GENDER_UNKNOWN;
        }

    }
    else if(0 == strcmp(text," "))
    {
        return;
    }
    else 
    {

        for(int i=0; i<=10;i++)
        {
            //to simplify the rel_list
            if( 0 == strcmp(text,btnm_relation[i].btnm_text))
            {
                if( (GENDER_FEMALE  == last_input.gender                    &&\
                     REL_WIFE       == btnm_relation[i].relation)               ||\
                    (GENDER_MALE == last_input.gender                         &&\
                     REL_HUSBAND == btnm_relation[i].relation)         
                  )
                {
                    return;
                }
                
                if( (REL_ELDER_BROTHER == last_input.relation                   &&\
                     REL_ELDER_BROTHER == btnm_relation[i].relation)            ||\
                    (REL_YOUNGER_BROTHER == last_input.relation                 &&\
                     REL_YOUNGER_BROTHER == btnm_relation[i].relation)          ||\
                    (REL_ELDER_SISTER == last_input.relation                    &&\
                     REL_ELDER_SISTER == btnm_relation[i].relation)             ||\
                    (REL_YOUNGER_SISTER == last_input.relation                  &&\
                     REL_YOUNGER_SISTER == btnm_relation[i].relation) 
                  )
                {
                    last_input.relation = REL_NULL;
                    last_input.gender = GENDER_UNKNOWN;
                    break;
                }else if( REL_HUSBAND == last_input.relation              &&\
                          REL_HUSBAND == btnm_relation[i].relation )
                {
                    last_input.relation = REL_NULL;
                    last_input.gender = GENDER_UNKNOWN;
                    break;
                }else if( (REL_YOUNGER_BROTHER == last_input.relation                   &&\
                           REL_ELDER_BROTHER == btnm_relation[i].relation)              ||\
                          (REL_ELDER_BROTHER == last_input.relation                     &&\
                           REL_YOUNGER_BROTHER == btnm_relation[i].relation)            ||\
                          (REL_YOUNGER_SISTER == last_input.relation                    &&\
                           REL_ELDER_SISTER == btnm_relation[i].relation)               ||\
                          (REL_ELDER_SISTER == last_input.relation                      &&\
                           REL_YOUNGER_SISTER == btnm_relation[i].relation)             ||\
                          (REL_WIFE == last_input.relation                              &&\
                           REL_HUSBAND == btnm_relation[i].relation)                    ||\
                          (REL_HUSBAND == last_input.relation                           &&\
                           REL_WIFE == btnm_relation[i].relation)                       ||\
                          (((REL_SON == last_input.relation)                            ||\
                            (REL_DAUGHTER == last_input.relation))                      &&\
                           ((REL_FATHER == btnm_relation[i].relation)                   ||\
                            (REL_MOTHER == btnm_relation[i].relation)))                 ||\
                          (((REL_FATHER == last_input.relation)                         ||\
                            (REL_MOTHER == last_input.relation))                        &&\
                           ((REL_SON == btnm_relation[i].relation)                      ||\
                            (REL_DAUGHTER == btnm_relation[i].relation)))
                        )/*TBD:simplify expressions*/
                {
                    last_input.relation = REL_NULL;
                    last_input.gender = GENDER_UNKNOWN;
                    delete_relation_node(&g_rel_cal);
                    break;
                }

                add_relation_node(btnm_relation[i].relation,&g_rel_cal);
                last_input = btnm_relation[i];
                break;
            }
        }

        if(g_rel_cal.status == REL_NODE_SUCCESS)
        {
            lv_textarea_set_text(g_rel_cal.ui.screen,"");
            g_rel_cal.status = REL_CALING;
        }

        lv_textarea_add_text(g_rel_cal.ui.screen, text);
        lv_textarea_add_text(g_rel_cal.ui.screen, "的");

    }
    
}


static uint8_t add_relation_node(relation_type_t relation,relation_cal_t *self)
{
    relation_node_t *node = (relation_node_t *)malloc(sizeof(relation_node_t));
    if( NULL == node)
    {
        return REL_NODE_ERROR;
    }
    node->relation = relation;
    node->next = self->rel_list;
    self->rel_list = node;
    return REL_NODE_SUCCESS;
}

static uint8_t clear_relation_node(relation_cal_t *self)
{
    if(NULL == self->rel_list && \
       NULL == self             )
    {
        return REL_NODE_ERROR;
    }

    while (self->rel_list)
    {
        relation_node_t *temp = self->rel_list;
        self->rel_list = self->rel_list->next;
        free(temp);
    }
    return REL_NODE_SUCCESS;
}

static uint8_t delete_relation_node(relation_cal_t *self)
{
    /*from head*/
    relation_node_t *temp = self->rel_list;
    if(temp)
    {
        self->rel_list = temp->next;
        free(temp);
    }else{
        return REL_NODE_ERROR;
    }
    return REL_NODE_SUCCESS;
}

static const char *calculate_relationship(relation_cal_t *self)
{
    relation_node_t *prev = NULL, *curr = self->rel_list;
    while (curr)
    {
        relation_node_t *next = curr->next;
        curr->next = prev;
        prev = curr;
        curr = next;
    }/*TBD:add node from tail no important*/
    
    const char *result = navigate_relationship(&self->mine,prev);

    return result;
}

static const char  *navigate_relationship(relation_tree_t *current,relation_node_t *rel)
{
    if (!rel)
    {
        return current->call;
    }
    switch(rel->relation) {
    case REL_FATHER:
        if(!current->father)
            return "未知";
        return navigate_relationship(current->father,rel->next);

    case REL_MOTHER:
        if(!current->mother)
            return "未知";
        return navigate_relationship(current->mother,rel->next);
    
    case REL_HUSBAND:
        if(!current->husband)
            return "未知";
        return navigate_relationship(current->husband,rel->next);

    case REL_WIFE:
        if(!current->wife)
            return "未知";
        return navigate_relationship(current->wife,rel->next);

    case REL_SON:
        if(!current->son)
            return "未知";
        return navigate_relationship(current->son,rel->next);
    
    case REL_DAUGHTER:
        if(!current->daughter)
            return "未知";
        return navigate_relationship(current->daughter,rel->next);

    case REL_ELDER_BROTHER:
        if(!current->elder_bro)
            return "未知";
        return navigate_relationship(current->elder_bro,rel->next);

    case REL_YOUNGER_BROTHER:
         if(!current->younger_bro)
            return "未知";
        return navigate_relationship(current->younger_bro,rel->next);

    case REL_ELDER_SISTER:
        if(!current->elder_sis)
            return "未知";
        return navigate_relationship(current->elder_sis,rel->next);

    case REL_YOUNGER_SISTER:
        if(!current->younger_sis)
            return "未知";
        return navigate_relationship(current->younger_sis,rel->next);

    default:
        return "未支持的关系";
    }
}


static uint8_t init_relation_tree(relation_tree_t *self, relation_tree_t *relation_buffer) 
{
    self->gender = GENDER_MALE;

    // Father's Line
    self->father = &relation_buffer[0];
    self->father->gender = GENDER_MALE;
    self->father->call = "父亲";

    // Paternal Grandparents
    self->father->mother = &relation_buffer[1];
    self->father->mother->gender = GENDER_FEMALE;
    self->father->mother->call = "奶奶"; 

    self->father->father = &relation_buffer[2];
    self->father->father->call = "爷爷";
    self->father->mother->gender = GENDER_MALE;

    // Paternal Great-grandparents
    self->father->father->mother = &relation_buffer[3];
    self->father->father->mother->call = "曾祖母";   
    self->father->father->mother->gender = GENDER_FEMALE;

    self->father->father->father = &relation_buffer[4];
    self->father->father->father->call = "曾祖父";
    self->father->father->father->gender = GENDER_MALE;

    // Paternal Great-great-grandparents (High Grandparents)
    self->father->father->father->mother = &relation_buffer[5];
    self->father->father->father->mother->call = "高祖母";
    self->father->father->father->mother->gender = GENDER_FEMALE;

    self->father->father->father->father = &relation_buffer[6];
    self->father->father->father->father->call = "高祖父";
    self->father->father->father->father->gender = GENDER_MALE;

    self->father->mother->mother = &relation_buffer[7];
    self->father->mother->mother->call = "曾外祖母"; 
    self->father->mother->mother->gender = GENDER_FEMALE;

    self->father->mother->father = &relation_buffer[8];
    self->father->mother->father->call = "曾外祖父"; 
    self->father->mother->father->gender = GENDER_MALE;

    // Mother's Line
    self->mother = &relation_buffer[9];
    self->mother->call = "母亲";
    self->mother->gender = GENDER_FEMALE;

    // Maternal Grandparents
    self->mother->mother = &relation_buffer[10];
    self->mother->mother->call = "外婆"; 
    self->mother->mother->gender = GENDER_FEMALE;

    self->mother->father = &relation_buffer[11];
    self->mother->father->call = "外公";
    self->mother->father->gender = GENDER_MALE;

    // Maternal Great-grandparents
    self->mother->mother->mother = &relation_buffer[12];
    self->mother->mother->mother->call = "外曾外祖母";
    self->mother->mother->mother->gender = GENDER_FEMALE;

    self->mother->mother->father = &relation_buffer[13];
    self->mother->mother->father->call = "外曾外祖父"; 
    self->mother->mother->father->gender = GENDER_MALE;

    self->mother->father->mother = &relation_buffer[14];
    self->mother->father->mother->call = "外曾祖母";   
    self->mother->father->mother->gender = GENDER_FEMALE;

    self->mother->father->father = &relation_buffer[15];
    self->mother->father->father->call = "外曾祖父";   
    self->mother->father->father->gender = GENDER_MALE;

    // Maternal Great-great-grandparents
    self->mother->mother->father->mother = &relation_buffer[16];
    self->mother->mother->father->mother->call = "外高祖母";
    self->mother->mother->father->mother->gender = GENDER_FEMALE;

    self->mother->mother->father->father = &relation_buffer[17];
    self->mother->mother->father->father->call = "外高祖父";
    self->mother->mother->father->father->gender = GENDER_MALE;

    // Spouse's Line
    self->wife = &relation_buffer[18];
    self->wife->call = "妻子";
    self->wife->gender = GENDER_FEMALE;

    self->husband = &relation_buffer[19];
    self->husband->call = "丈夫";
    self->husband->gender = GENDER_MALE;

    // Spouse's Parents
    self->wife->father = &relation_buffer[20];
    self->wife->father->call = "岳父   ";
    self->wife->father->gender = GENDER_MALE;

    self->wife->mother = &relation_buffer[21];
    self->wife->mother->call = "岳母";
    self->wife->mother->gender = GENDER_FEMALE;

    self->husband->father = &relation_buffer[22];
    self->husband->father->call = "公公";
    self->husband->father->gender = GENDER_MALE;

    self->husband->mother = &relation_buffer[23];
    self->husband->mother->call = "婆婆";
    self->husband->mother->gender = GENDER_FEMALE;

    // Children
    self->son = &relation_buffer[24];
    self->son->call = "儿子";
    self->son->gender = GENDER_MALE;

    self->daughter = &relation_buffer[25];
    self->daughter->call = "女儿";
    self->daughter->gender = GENDER_FEMALE;

    // Grandchildren
    self->son->son = &relation_buffer[26];
    self->son->son->call = "孙子";
    self->son->son->gender = GENDER_MALE;

    self->son->daughter = &relation_buffer[27];
    self->son->daughter->call = "孙女";
    self->son->daughter->gender = GENDER_FEMALE;

    self->daughter->son = &relation_buffer[28];
    self->daughter->son->call = "外孙";
    self->daughter->son->gender = GENDER_MALE;

    self->daughter->daughter = &relation_buffer[29];
    self->daughter->daughter->call = "外孙女";
    self->daughter->daughter->gender = GENDER_FEMALE;

    // Great-grandchildren
    self->son->son->son = &relation_buffer[30];
    self->son->son->son->call = "曾孙";
    self->son->son->son->gender = GENDER_MALE;

    self->son->son->daughter = &relation_buffer[31];
    self->son->son->daughter->call = "曾孙女";
    self->son->son->daughter->gender = GENDER_FEMALE;

    self->son->daughter->son = &relation_buffer[32];
    self->son->daughter->son->call = "曾孙";
    self->son->daughter->son->gender = GENDER_MALE;

    self->son->daughter->daughter = &relation_buffer[33];
    self->son->daughter->daughter->call = "曾孙女";
    self->son->daughter->daughter->gender = GENDER_FEMALE;

    self->daughter->son->son = &relation_buffer[34];
    self->daughter->son->son->call = "曾外孙";
    self->daughter->son->son->gender = GENDER_MALE;

    self->daughter->son->daughter = &relation_buffer[35];
    self->daughter->son->daughter->call = "曾外孙女";
    self->daughter->son->daughter->gender = GENDER_FEMALE;

    self->daughter->daughter->son = &relation_buffer[36];
    self->daughter->daughter->son->call = "曾外孙";
    self->daughter->daughter->son->gender = GENDER_MALE;

    self->daughter->daughter->daughter = &relation_buffer[37];
    self->daughter->daughter->daughter->call = "曾外孙女";
    self->daughter->daughter->daughter->gender = GENDER_FEMALE;

    // Spouse's Siblings
    self->wife->elder_bro = &relation_buffer[38];
    self->wife->elder_bro->call = "大舅子"; 
    self->wife->elder_bro->gender = GENDER_MALE;

    self->wife->younger_bro = &relation_buffer[39];
    self->wife->younger_bro->call = "小舅子"; 
    self->wife->younger_bro->gender = GENDER_MALE;

    self->wife->elder_sis = &relation_buffer[40];
    self->wife->elder_sis->call = "大姨子"; // Elder Sister-in-law
    self->wife->elder_sis->gender = GENDER_FEMALE;

    self->wife->younger_sis = &relation_buffer[41];
    self->wife->younger_sis->call = "小姨子"; // Younger Sister-in-law
    self->wife->younger_sis->gender = GENDER_FEMALE;

    // Children's Spouses
    self->son->wife = &relation_buffer[42];
    self->son->wife->call = "儿媳";
    self->son->wife->gender = GENDER_FEMALE;

    self->daughter->husband = &relation_buffer[43];
    self->daughter->husband->call = "女婿";
    self->daughter->husband->gender = GENDER_MALE;

    // Siblings
    self->elder_bro = &relation_buffer[44];
    self->elder_bro->call = "哥哥";
    self->elder_bro->gender = GENDER_MALE;

    self->younger_bro = &relation_buffer[45];
    self->younger_bro->call = "弟弟";
    self->younger_bro->gender = GENDER_MALE;

    self->elder_sis = &relation_buffer[46];
    self->elder_sis->call = "姐姐";
    self->elder_sis->gender = GENDER_FEMALE;

    self->younger_sis = &relation_buffer[47];
    self->younger_sis->call = "妹妹";
    self->younger_sis->gender = GENDER_FEMALE;

    // Siblings' Families
    self->elder_bro->wife = &relation_buffer[48];
    self->elder_bro->wife->call = "嫂子";
    self->elder_bro->wife->gender = GENDER_FEMALE;

    self->younger_bro->wife = &relation_buffer[49];
    self->younger_bro->wife->call = "弟媳";
    self->younger_bro->wife->gender = GENDER_FEMALE;

    self->elder_sis->husband = &relation_buffer[50];
    self->elder_sis->husband->call = "姐夫";
    self->elder_sis->husband->gender = GENDER_MALE;

    self->younger_sis->husband = &relation_buffer[51];
    self->younger_sis->husband->call = "妹夫";
    self->younger_sis->husband->gender = GENDER_MALE;

    // Nephews and Nieces
    self->elder_bro->son = &relation_buffer[52];
    self->elder_bro->son->call = "侄子";
    self->elder_bro->son->gender = GENDER_MALE;

    self->elder_bro->daughter = &relation_buffer[53];
    self->elder_bro->daughter->call = "侄女";
    self->elder_bro->daughter->gender = GENDER_FEMALE;

    self->younger_bro->son = &relation_buffer[54];
    self->younger_bro->son->call = "侄子";
    self->younger_bro->son->gender = GENDER_MALE;

    self->younger_bro->daughter = &relation_buffer[55];
    self->younger_bro->daughter->call = "侄女";
    self->younger_bro->daughter->gender = GENDER_FEMALE;

    self->elder_sis->son = &relation_buffer[56];
    self->elder_sis->son->call = "外甥";
    self->elder_sis->son->gender = GENDER_MALE;

    self->elder_sis->daughter = &relation_buffer[57];
    self->elder_sis->daughter->call = "外甥女";
    self->elder_sis->daughter->gender = GENDER_FEMALE;

    self->younger_sis->son = &relation_buffer[58];
    self->younger_sis->son->call = "外甥";
    self->younger_sis->son->gender = GENDER_MALE;

    self->younger_sis->daughter = &relation_buffer[59];
    self->younger_sis->daughter->call = "外甥女";
    self->younger_sis->daughter->gender = GENDER_FEMALE;

    // Mother's Siblings
    self->mother->elder_bro = &relation_buffer[60];
    self->mother->elder_bro->call = "大舅舅"; 
    self->mother->elder_bro->gender = GENDER_MALE;

    self->mother->younger_bro = &relation_buffer[61];
    self->mother->younger_bro->call = "小舅舅"; 
    self->mother->younger_bro->gender = GENDER_MALE;

    self->mother->elder_sis = &relation_buffer[62];
    self->mother->elder_sis->call = "大姑"; 
    self->mother->elder_sis->gender = GENDER_FEMALE;

    self->mother->younger_sis = &relation_buffer[63];
    self->mother->younger_sis->call = "小姑"; 
    self->mother->younger_sis->gender = GENDER_FEMALE;

    // Mother's Siblings' Families
    self->mother->elder_bro->wife = &relation_buffer[64];
    self->mother->elder_bro->wife->call = "大舅母"; 
    self->mother->elder_bro->wife->gender = GENDER_FEMALE;

    self->mother->younger_bro->wife = &relation_buffer[65];
    self->mother->younger_bro->wife->call = "小舅母"; 
    self->mother->younger_bro->wife->gender = GENDER_FEMALE;

    self->mother->elder_sis->husband = &relation_buffer[66];
    self->mother->elder_sis->husband->call = "大姑父"; 
    self->mother->elder_sis->husband->gender = GENDER_MALE;

    self->mother->younger_sis->husband = &relation_buffer[67];
    self->mother->younger_sis->husband->call = "小姑父"; 
    self->mother->younger_sis->husband->gender = GENDER_MALE;

    // Father's Siblings
    self->father->elder_bro = &relation_buffer[68];
    self->father->elder_bro->call = "伯父";
    self->father->elder_bro->gender = GENDER_MALE;

    self->father->younger_bro = &relation_buffer[69];
    self->father->younger_bro->call = "叔父";
    self->father->younger_bro->gender = GENDER_MALE;

    self->father->elder_sis = &relation_buffer[70];
    self->father->elder_sis->call = "姑妈";
    self->father->elder_sis->gender = GENDER_FEMALE;

    self->father->younger_sis = &relation_buffer[71];
    self->father->younger_sis->call = "姑妈";
    self->father->younger_sis->gender = GENDER_FEMALE;

    // Father's Siblings' Families
    self->father->elder_bro->wife = &relation_buffer[72];
    self->father->elder_bro->wife->call = "伯母";
    self->father->elder_bro->wife->gender = GENDER_FEMALE;

    self->father->younger_bro->wife = &relation_buffer[73];
    self->father->younger_bro->wife->call = "婶婶";
    self->father->younger_bro->wife->gender = GENDER_FEMALE;

    self->father->elder_sis->husband = &relation_buffer[74];
    self->father->elder_sis->husband->call = "姑父";
    self->father->elder_sis->husband->gender = GENDER_MALE;

    self->father->younger_sis->husband = &relation_buffer[75];
    self->father->younger_sis->husband->call = "姑父";
    self->father->younger_sis->husband->gender = GENDER_MALE;

    // Cousins (Father's Side)
    self->father->elder_bro->son = &relation_buffer[76];
    self->father->elder_bro->son->call = "堂哥";
    self->father->elder_bro->son->gender = GENDER_MALE;

    self->father->elder_bro->daughter = &relation_buffer[77];
    self->father->elder_bro->daughter->call = "堂姐";
    self->father->elder_bro->daughter->gender = GENDER_FEMALE;

    self->father->younger_bro->son = &relation_buffer[78];
    self->father->younger_bro->son->call = "堂弟";
    self->father->younger_bro->son->gender = GENDER_MALE;

    self->father->younger_bro->daughter = &relation_buffer[79];
    self->father->younger_bro->daughter->call = "堂妹";
    self->father->younger_bro->daughter->gender = GENDER_FEMALE;

    // Cousins (Mother's Side)
    self->mother->elder_bro->son = &relation_buffer[80];
    self->mother->elder_bro->son->call = "表哥";
    self->mother->elder_bro->son->gender = GENDER_MALE;

    self->mother->elder_bro->daughter = &relation_buffer[81];
    self->mother->elder_bro->daughter->call = "表姐";
    self->mother->elder_bro->daughter->gender = GENDER_FEMALE;

    self->mother->younger_bro->son = &relation_buffer[82];
    self->mother->younger_bro->son->call = "表弟";
    self->mother->younger_bro->son->gender = GENDER_MALE;

    self->mother->younger_bro->daughter = &relation_buffer[83];
    self->mother->younger_bro->daughter->call = "表妹";
    self->mother->younger_bro->daughter->gender = GENDER_FEMALE;

    self->mother->elder_sis->son = &relation_buffer[84];
    self->mother->elder_sis->son->call = "表哥";
    self->mother->elder_sis->son->gender = GENDER_MALE;

    self->mother->elder_sis->daughter = &relation_buffer[85];
    self->mother->elder_sis->daughter->call = "表姐";
    self->mother->elder_sis->daughter->gender = GENDER_FEMALE;

    self->mother->younger_sis->son = &relation_buffer[86];
    self->mother->younger_sis->son->call = "表弟";
    self->mother->younger_sis->son->gender = GENDER_MALE;

    self->mother->younger_sis->daughter = &relation_buffer[87];
    self->mother->younger_sis->daughter->call = "表妹";
    self->mother->younger_sis->daughter->gender = GENDER_FEMALE;

    // Grandchildren's Spouses
    self->son->son->wife = &relation_buffer[88];
    self->son->son->wife->call = "孙媳";
    self->son->son->wife->gender = GENDER_FEMALE;

    self->son->daughter->husband = &relation_buffer[89];
    self->son->daughter->husband->call = "孙婿";
    self->son->daughter->husband->gender = GENDER_MALE;

    self->daughter->son->wife = &relation_buffer[90];
    self->daughter->son->wife->call = "外孙媳";
    self->daughter->son->wife->gender = GENDER_FEMALE;

    self->daughter->daughter->husband = &relation_buffer[91];
    self->daughter->daughter->husband->call = "外孙婿";
    self->daughter->daughter->husband->gender = GENDER_MALE;

    // Great-grandchildren
    self->son->son->son = &relation_buffer[92];
    self->son->son->son->call = "曾孙";
    self->son->son->son->gender = GENDER_MALE;

    self->son->son->daughter = &relation_buffer[93];
    self->son->son->daughter->call = "曾孙女";
    self->son->daughter->daughter->gender = GENDER_FEMALE;

    self->son->daughter->son = &relation_buffer[94];
    self->son->daughter->son->call = "曾孙";
    self->son->daughter->son->gender = GENDER_MALE;

    self->son->daughter->daughter = &relation_buffer[95];
    self->son->daughter->daughter->call = "曾孙女";
    self->son->daughter->daughter->gender = GENDER_FEMALE;

    self->daughter->son->son = &relation_buffer[96];
    self->daughter->son->son->call = "曾外孙";
    self->daughter->son->son->gender = GENDER_MALE;

    self->daughter->son->daughter = &relation_buffer[97];
    self->daughter->son->daughter->call = "曾外孙女";
    self->daughter->son->daughter->gender = GENDER_FEMALE;

    self->daughter->daughter->son = &relation_buffer[98];
    self->daughter->daughter->son->call = "曾外孙";
    self->daughter->daughter->son->gender = GENDER_MALE;

    self->daughter->daughter->daughter = &relation_buffer[99];
    self->daughter->daughter->daughter->call = "曾外孙女";
    self->daughter->daughter->daughter->gender = GENDER_FEMALE;

    // Grandchildren's Grandchildren
    self->son->son->son->son = &relation_buffer[100];
    self->son->son->son->son->call = "玄孙";
    self->son->son->son->son->gender = GENDER_MALE;

    self->son->son->son->daughter = &relation_buffer[101];
    self->son->son->son->daughter->call = "玄孙女";
    self->son->son->son->daughter->gender = GENDER_FEMALE;

    self->son->son->daughter->son = &relation_buffer[102];
    self->son->son->daughter->son->call = "玄孙";
    self->son->son->daughter->son->gender = GENDER_MALE;

    self->son->son->daughter->daughter = &relation_buffer[103];
    self->son->son->daughter->daughter->call = "玄孙女";
    self->son->daughter->daughter->gender = GENDER_FEMALE;

    // Grandchildren's Grandchildren
    self->son->son->son = &relation_buffer[104];
    self->son->son->son->call = "玄孙";
    self->son->son->son->gender = GENDER_MALE;
    
    self->son->son->daughter = &relation_buffer[105];
    self->son->son->daughter->call = "玄孙女";
    self->son->son->daughter->gender = GENDER_FEMALE;
    
    self->son->daughter->son = &relation_buffer[106];
    self->son->daughter->son->call = "玄孙";
    self->son->daughter->son->gender = GENDER_MALE;
    
    self->son->daughter->daughter = &relation_buffer[107];
    self->son->daughter->daughter->call = "玄孙女";
    self->son->daughter->daughter->gender = GENDER_FEMALE;
    
    self->daughter->son->son = &relation_buffer[108];
    self->daughter->son->son->call = "玄外孙";
    self->daughter->son->son->gender = GENDER_MALE;
    
    self->daughter->son->daughter = &relation_buffer[109];
    self->daughter->son->daughter->call = "玄外孙女";
    self->daughter->son->daughter->gender = GENDER_FEMALE;
    
    self->daughter->daughter->son = &relation_buffer[110];
    self->daughter->daughter->son->call = "玄外孙";
    self->daughter->daughter->son->gender = GENDER_MALE;
    
    self->daughter->daughter->daughter = &relation_buffer[111];
    self->daughter->daughter->daughter->call = "玄外孙女";
    self->daughter->daughter->daughter->gender = GENDER_FEMALE;
    
    // Grandchildren's Great-grandchildren
    self->son->son->son->son = &relation_buffer[112];
    self->son->son->son->son->call = "五世孙";
    self->son->son->son->son->gender = GENDER_MALE;
    
    self->son->son->son->daughter = &relation_buffer[113];
    self->son->son->son->daughter->call = "五世孙女";
    self->son->son->son->daughter->gender = GENDER_FEMALE;
    
    self->son->son->daughter->son = &relation_buffer[114];
    self->son->son->daughter->son->call = "五世孙";
    self->son->son->daughter->son->gender = GENDER_MALE;
    
    self->son->son->daughter->daughter = &relation_buffer[115];
    self->son->son->daughter->daughter->call = "五世孙女";
    self->son->son->daughter->daughter->gender = GENDER_FEMALE;
    
    self->son->daughter->son->son = &relation_buffer[116];
    self->son->daughter->son->son->call = "五世孙";
    self->son->daughter->son->son->gender = GENDER_MALE;
    
    self->son->daughter->son->daughter = &relation_buffer[117];
    self->son->daughter->son->daughter->call = "五世孙女";
    self->son->daughter->son->daughter->gender = GENDER_FEMALE;
    
    self->son->daughter->daughter->son = &relation_buffer[118];
    self->son->daughter->daughter->son->call = "五世孙";
    self->son->daughter->daughter->son->gender = GENDER_MALE;
    
    self->son->daughter->daughter->daughter = &relation_buffer[119];
    self->son->daughter->daughter->daughter->call = "五世孙女";
    self->son->daughter->daughter->daughter->gender = GENDER_FEMALE;
    
    self->daughter->son->son->son = &relation_buffer[120];
    self->daughter->son->son->son->call = "五世外孙";
    self->daughter->son->son->son->gender = GENDER_MALE;
    
    self->daughter->son->son->daughter = &relation_buffer[121];
    self->daughter->son->son->daughter->call = "五世外孙女";
    self->daughter->son->son->daughter->gender = GENDER_FEMALE;
    
    self->daughter->son->daughter->son = &relation_buffer[122];
    self->daughter->son->daughter->son->call = "五世外孙";
    self->daughter->son->daughter->son->gender = GENDER_MALE;
    
    self->daughter->son->daughter->daughter = &relation_buffer[123];
    self->daughter->son->daughter->daughter->call = "五世外孙女";
    self->daughter->son->daughter->daughter->gender = GENDER_FEMALE;
    
    self->daughter->daughter->son->son = &relation_buffer[124];
    self->daughter->daughter->son->son->call = "五世外孙";
    self->daughter->daughter->son->son->gender = GENDER_MALE;
    
    self->daughter->daughter->son->daughter = &relation_buffer[125];
    self->daughter->daughter->son->daughter->call = "五世外孙女";
    self->daughter->daughter->son->daughter->gender = GENDER_FEMALE;
    
    self->daughter->daughter->daughter->son = &relation_buffer[126];
    self->daughter->daughter->daughter->son->call = "五世外孙";
    self->daughter->daughter->daughter->son->gender = GENDER_MALE;
    
    self->daughter->daughter->daughter->daughter = &relation_buffer[127];
    self->daughter->daughter->daughter->daughter->call = "五世外孙女";
    self->daughter->daughter->daughter->daughter->gender = GENDER_FEMALE;
    
    // Add parents for all newly added generations
    self->son->son->father = self->son;
    self->son->daughter->father = self->son;
    self->daughter->son->father = self->daughter;
    self->daughter->daughter->father = self->daughter;
    
    self->son->son->mother = &relation_buffer[128];
    self->son->son->mother->call = "孙媳";
    self->son->son->mother->gender = GENDER_FEMALE;
    
    self->son->daughter->mother = &relation_buffer[129];
    self->son->daughter->mother->call = "孙婿";
    self->son->daughter->mother->gender = GENDER_MALE;
    
    self->daughter->son->mother = &relation_buffer[130];
    self->daughter->son->mother->call = "外孙媳";
    self->daughter->son->mother->gender = GENDER_FEMALE;
    
    self->daughter->daughter->mother = &relation_buffer[134];
    self->daughter->daughter->mother->call = "外孙婿";
    self->daughter->daughter->mother->gender = GENDER_MALE;
      
    self->mother->husband = &relation_buffer[144];
    self->mother->husband->call = "父亲";
    self->mother->husband->gender = GENDER_MALE;

    self->father->wife = &relation_buffer[143];
    self->father->wife->call = "母亲";
    self->father->wife->gender = GENDER_FEMALE;

    self->son->elder_bro = &relation_buffer[136];
    self->son->elder_bro->call = "儿子";
    self->son->elder_bro->gender = GENDER_MALE;

    self->son->younger_bro = &relation_buffer[137];
    self->son->younger_bro->call = "儿子";
    self->son->younger_bro->gender = GENDER_MALE;

    self->daughter->elder_sis = &relation_buffer[138];
    self->daughter->elder_sis->call = "女儿";
    self->daughter->elder_sis->gender = GENDER_FEMALE;

    self->daughter->younger_sis = &relation_buffer[139];
    self->daughter->younger_sis->call = "女儿";
    self->daughter->younger_sis->gender = GENDER_FEMALE;

    self->father->wife = self->mother;  
    self->mother->husband = self->father;

    self->wife->elder_bro->son = &relation_buffer[140];
    self->wife->elder_bro->son->call = "甥男"; 
    self->wife->elder_bro->son->gender = GENDER_MALE;

    self->wife->elder_bro->daughter = &relation_buffer[141];
    self->wife->elder_bro->daughter->call = "甥女";
    self->wife->elder_bro->daughter->gender = GENDER_FEMALE;

    self->daughter->daughter->mother = self->daughter; 

    self->daughter->daughter->husband = &relation_buffer[135];
    self->daughter->daughter->husband->call = "外孙婿";
    self->daughter->daughter->husband->gender = GENDER_MALE;

    self->son->son->son->wife = &relation_buffer[142];
    self->son->son->son->wife->call = "曾孙媳";
    self->son->son->son->wife->gender = GENDER_FEMALE;

    return REL_NODE_SUCCESS;
}


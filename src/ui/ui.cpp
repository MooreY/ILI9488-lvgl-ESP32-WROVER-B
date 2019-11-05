#include "ui.h"

void UI_Object::set_value(const std::string value)  {
    switch(type_) {
        case 1: //label
            lv_label_set_static_text(object_, value.c_str());
            break;
        case 2: //switch
            if(value.rfind("ON") == 0)
                lv_sw_on(object_,true);
            else
                lv_sw_off(object_,true);
            break;          
            
    }
}

UI_Object *UI::add_ui_object(lv_obj_t * object, int id ) {
    UI_Object *obj = new UI_Object(object, id);
    this->object_list_.push_back(obj);
    return obj;
}

UI_Object *UI::add_ui_object(int id, int type, lv_obj_t *parent, std::function<void(int)> callback, char* label) {
    lv_obj_t *lv_obj = nullptr;
    switch(type) {
        case 1: //label
            lv_obj = lv_label_create(parent,nullptr); 
            lv_label_set_static_text(lv_obj, label);
            break;
        case 2: //switch
            lv_obj = lv_sw_create(parent,nullptr);
            break;
    }
    if(lv_obj != nullptr) {
        UI_Object *ui_obj = new UI_Object(lv_obj,id);
        ui_obj->set_callback(callback);
    }
    return nullptr;
}

UI_Object *UI::find_object_by_id(int id) {
    for(auto *obj: object_list_) {
        if(obj->get_id() == id)
            return obj;
    }
    return nullptr;
}
#include <lvgl/lvgl.h>
#include <vector>
#include <functional>
#include <string>

//using ui_callback_t = std::function<void(UI_Object &)>;

class UI_Object;

class UI_Object {
public:
    UI_Object(lv_obj_t *object, int id){
        this->id_ = id;
        this->object_ = object;
    }
    void create_object(lv_obj_t *object);
    void set_callback(std::function<void(int)> callback) {
        this->callback_ = callback;
    }
    void set_value(const std::string);
    int get_id() {
        return this->id_;
    }
    lv_obj_t *get_lv_obj() {
        return object_;
    }
protected:
    int id_{0};
    int type_{0};
    lv_obj_t *object_;
    std::function<void(int)> callback_;
};


class UI {
public:
    UI_Object *add_ui_object(int id, int type, lv_obj_t *parent, std::function<void(int)> callback, char* label);
    UI_Object *add_ui_object(lv_obj_t * object, int id );
    UI_Object *find_object_by_id(int id);

protected:
    std::vector<UI_Object *> object_list_ ;

};
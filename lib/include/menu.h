#include "common.h"
#include "screen.h"
#include "keyboard.h"
#include <vector>

class Menu
{
    private:

        std::string title;  
        std::vector<std::string> data;
        uint8_t selection;
        uint8_t itemStartIndx;
        int lastIndex;
        uint8_t fontHeight;
        uint8_t width;
        uint8_t length;
        uint8_t displayItems;
        bool show;

    public:
        Menu(std::vector<std::string> menuDat,uint8_t w);
        ~Menu();

        void update(uint16_t deltaTimeMS);
        void draw(Display *display);
        void changeSelection(uint8_t dirctn);
        void showMenu(bool sm);
        uint8_t getSelection();
};
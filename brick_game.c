#include "brick_game.h"
#include <time.h>

int main() {
    FrontendContext *ctx = init_ncurses();
    if (ctx != NULL) {
        srand(time(NULL));
    
        GameInfo_t game_info = {0};
        init_game_info(&game_info);
    
        bool run_game = true;
    
        while (run_game) {
            int ch = getch();
            if (ch != ERR) {
                UserAction_t action = resolve_input(ch);
                if (action == Terminate) {
                    run_game = false;
                    continue;
                }
                userInput(action);
            }
    
            game_info = updateCurrentState();
            rendering(ctx, &game_info);
    
            napms(50);
        }
    
        free_matrix(FIELD_H, game_info.field);
        endwin();
        free(ctx);
    }
  
    return 0;
}

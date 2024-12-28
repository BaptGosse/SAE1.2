#include "board.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

int direction_list_classic[8][2] = {{1, -1},{1, 0},{1, 1},{0, -1},{0, 1},{-1, -1},{-1, 0},{-1, 1}};
int direction_list_hexa[8][2] = {{1, 0},{0, 0},{1, 1},{0, -1},{0, 1},{-1, -1},{0, 0},{-1, 0}};

enum range_types {KING, CELL_AVAILABLE, EMPTY_CELL, DEAD};

enum range_types range_tab[MAX_DIMENSION][MAX_DIMENSION];

struct board_s{
    cell cell_tab[MAX_DIMENSION][MAX_DIMENSION];
    player current_player;
    bool is_hex;
    bool uses_range;
    bool have_move;
};

board new_classic_game(bool use_range){
    board new_board = malloc(sizeof(struct board_s));
    if(!new_board)return NULL;
    for(int i = 0; i < MAX_DIMENSION; i++){
        for (int j = 0; j < MAX_DIMENSION; j++)
        {
            if(i < NB_LINES && j < NB_COLS){
                new_board->cell_tab[i][j]=EMPTY;
            }
            else{
                new_board->cell_tab[i][j]=KILLED;
            }
        }
    }
    new_board->cell_tab[0][NB_COLS/2] = NORTH;
    new_board->cell_tab[NB_LINES-1][NB_COLS/2] = SOUTH;
    new_board->is_hex = false;
    new_board->uses_range = use_range;
    new_board->have_move = false;
    new_board->current_player = NORTH;
    return new_board;
}

board new_hex_game(bool use_range){
    board new_board = malloc(sizeof(struct board_s));
    if(!new_board)return NULL;
    for(int i = 0; i < MAX_DIMENSION; i++){
        for (int j = 0; j < MAX_DIMENSION; j++)
        {
            if((j > MAX_DIMENSION/2+i && MAX_DIMENSION/2+i < MAX_DIMENSION)||(i > MAX_DIMENSION/2+j && MAX_DIMENSION/2+j < MAX_DIMENSION)){
                new_board->cell_tab[i][j]=KILLED;
            }
            else{
                new_board->cell_tab[i][j]=EMPTY;
            }
        }
    }
    new_board->cell_tab[0][2] = NORTH;
    new_board->cell_tab[8][6] = SOUTH;
    new_board->is_hex = true;
    new_board->uses_range = use_range;
    new_board->have_move = false;
    new_board->current_player = NORTH;
    return new_board;
}

board new_game(){
    return new_special_game(false, false);
}

board new_special_game(bool is_hex, bool use_range){
    if(use_range && !is_hex)return new_classic_game(true);
    if(!use_range && is_hex)return new_hex_game(false);
    if(use_range && is_hex)return new_hex_game(true);
    return new_classic_game(false);
}

board copy_game(board original_game){
    board new_board = malloc(sizeof(struct board_s));
    if(!new_board)return NULL;
    for(int i = 0; i < MAX_DIMENSION; i++){
        for(int j = 0; j < MAX_DIMENSION; j++){
            new_board->cell_tab[i][j] = original_game->cell_tab[i][j];
        }
    }
    new_board->current_player = original_game->current_player;
    new_board->is_hex = original_game->is_hex;
    new_board->uses_range = original_game->uses_range;
    return new_board;
}

void destroy_game(board game){
    free(game);
}

bool is_hex(board game){
    return game->is_hex;
}

bool uses_range(board game){
    return game->uses_range;
}

player current_player(board game){
    return game->current_player;
}

cell get_content(board game, int line, int column){
    if(line < 0 || column < 0 || line > MAX_DIMENSION || column > MAX_DIMENSION)return KILLED;
    return game->cell_tab[line][column];
}

void find_player(board game, player search_player, int * line, int * column){
    for(int i = 0; i < MAX_DIMENSION; i++){
        for(int j = 0; j < MAX_DIMENSION; j++){
            cell cell_content = get_content(game, i, j);
            if(cell_content == NORTH_KING || cell_content == SOUTH_KING){
                if (cell_content == NORTH_KING && search_player == NORTH)
                {
                    *line=i;
                    *column=j;
                }
                if (cell_content == SOUTH_KING && search_player == SOUTH)
                {
                    *line=i;
                    *column=j;
                }
            }
        }
    }
}

void add_in_pos_available(int pos_available[8][2], int * taille, int line, int column){
    pos_available[*taille][0] = line;
    pos_available[*taille][1] = column;
    *taille = *taille + 1;
}

void get_pos_arround(int pos_available[8][2], int * taille, int line_player, int column_player, bool is_hex){
    for(int i = 0; i < 8; i++){
        int direction_x, direction_y;
        if(!is_hex){
            direction_x = direction_list_classic[i][0];
            direction_y = direction_list_classic[i][1];
        }
        else{
            direction_x = direction_list_hexa[i][0];
            direction_y = direction_list_hexa[i][1];
        }

        if(line_player+direction_x < MAX_DIMENSION && column_player+direction_y < MAX_DIMENSION){
            if((is_hex && (direction_x != 0 || direction_y != 0)) || !is_hex){
                add_in_pos_available(pos_available, taille, line_player+direction_x, column_player+direction_y);
            }
        }
    }
}

enum return_code test_pos_arround(board game, int pos_available[8][2], int taille){
    for(int i = 0; i < taille; i++){
        if(get_content(game, pos_available[i][0], pos_available[i][1]) == EMPTY){
            return OK;
        }
    }
    return OUT;
}

player get_winner(board game){
    int pos_available[8][2];
    int taille = 0;
    int line_player, column_player;
    find_player(game, current_player(game), &line_player, &column_player);
    get_pos_arround(pos_available, &taille, line_player, column_player, is_hex(game));
    if(test_pos_arround(game, pos_available, taille) == OK) return NO_PLAYER;
    if(current_player(game) == NORTH){
        return SOUTH;
    }
    return NORTH;
}

void get_direction_classic(direction direction, int * line, int * column){
    *line = direction_list_classic[direction][0];
    *column = direction_list_classic[direction][1];
}

void get_direction_hexa(direction direction, int * line, int * column){
    *line = direction_list_hexa[direction][0];
    *column = direction_list_hexa[direction][1];
}

enum return_code translate_direction_classic(direction direction, int * line, int * column){
    get_direction_classic(direction, line, column);
    return OK;
}

enum return_code translate_direction_hexa(direction direction, int * line, int * column){
    if(direction == N || direction == S)return RULES;
    get_direction_hexa(direction, line, column);
    return OK;
}

enum return_code translate_direction(direction direction, bool is_hex, int * line, int * column){
    if(is_hex){
        return translate_direction_hexa(direction, line, column);
    }
    return translate_direction_classic(direction, line, column);
}

enum return_code move_toward(board game, direction direction){
    if(game->have_move)return RULES;
    if(direction > NE || direction < SW) return RULES;

    int add_line_move, add_column_move, last_line, last_column, next_line, next_column;;

    if(translate_direction(direction, game->is_hex, &add_line_move, &add_column_move) != OK) return RULES;

    find_player(game, current_player(game), &last_line, &last_column);

    next_line = last_line+add_line_move;
    next_column = last_column+add_column_move;

    if(next_line < 0 || next_line >= MAX_DIMENSION || next_column < 0 || next_column >= MAX_DIMENSION) return OUT;

    cell cell_content = get_content(game, next_line, next_column);

    if(cell_content == KILLED) return OUT;
    if(cell_content == NORTH_KING || cell_content == SOUTH_KING) return BUSY;

    game->cell_tab[last_line][last_column] = EMPTY;

    if(current_player(game) == NORTH) game->cell_tab[next_line][next_column] = NORTH_KING;
    else if(current_player(game) == SOUTH) game->cell_tab[next_line][next_column] = SOUTH_KING;

    game->have_move = true;

    return OK;
}

void put_case_3(int line, int column, bool is_hex){
    for(int i = 0; i < 8; i++){
        int line_c3, column_c3;

        if(!is_hex){
            line_c3 = line + direction_list_classic[i][0];
            column_c3 = column + direction_list_classic[i][1];
        }
        else{
            line_c3 = line + direction_list_hexa[i][0];
            column_c3 = column + direction_list_hexa[i][1];
        }

        if(line_c3 >= 0 && line_c3 <= MAX_DIMENSION && column_c3 >= 0 && column_c3 <= MAX_DIMENSION){
            if(!is_hex || (is_hex && (line_c3 != line || column_c3 != column))){
                if(range_tab[line_c3][column_c3] == EMPTY_CELL){
                    range_tab[line_c3][column_c3] = CELL_AVAILABLE;
                }
            }
        }
    }
}

void put_case_2(int line, int column, bool is_hex){
    for(int i = 0; i < 8; i++){
        int line_c2, column_c2;

        if(!is_hex){
            line_c2 = line + direction_list_classic[i][0];
            column_c2 = column + direction_list_classic[i][1];
        }
        else{
            line_c2 = line + direction_list_hexa[i][0];
            column_c2 = column + direction_list_hexa[i][1];
        }

        if(line_c2 >= 0 && line_c2 <= MAX_DIMENSION && column_c2 >= 0 && column_c2 <= MAX_DIMENSION){
            if(!is_hex || (is_hex && (line_c2 != line || column_c2 != column))){
                if(range_tab[line_c2][column_c2] == EMPTY_CELL || range_tab[line_c2][column_c2] == CELL_AVAILABLE){
                    range_tab[line_c2][column_c2] = CELL_AVAILABLE;
                    put_case_3(line_c2, column_c2, is_hex);
                }
            }
        }
    }
}

void put_case_1(int line, int column, bool is_hex){
    for(int i = 0; i < 8; i++){
        int line_c1, column_c1;

        if(!is_hex){
            line_c1 = line + direction_list_classic[i][0];
            column_c1 = column + direction_list_classic[i][1];
        }
        else{
            line_c1 = line + direction_list_hexa[i][0];
            column_c1 = column + direction_list_hexa[i][1];
        }

        if(line_c1 >= 0 && line_c1 <= MAX_DIMENSION && column_c1 >= 0 && column_c1 <= MAX_DIMENSION){
            if(!is_hex || (is_hex && (line_c1 != line || column_c1 != column))){
                if(range_tab[line_c1][column_c1] == EMPTY_CELL || range_tab[line_c1][column_c1] == CELL_AVAILABLE){
                    range_tab[line_c1][column_c1] = CELL_AVAILABLE;
                    put_case_2(line_c1, column_c1, is_hex);
                }
            }
        }
    }
}

void copy_in_range(board game, int player_line, int player_column, bool is_hex){
    for(int i = 0; i < MAX_DIMENSION; i++){
        for(int j = 0; j < MAX_DIMENSION; j++){
            if(get_content(game, i, j) != EMPTY) range_tab[i][j] = DEAD;
            else range_tab[i][j] = EMPTY_CELL;
        }
    }
    range_tab[player_line][player_column] = KING;
    put_case_1(player_line, player_column, is_hex);
}

enum return_code kill_cell(board game, int line, int column){
    if(!game->have_move)return RULES;

    if(line < 0 || column < 0 || line > MAX_DIMENSION || column > MAX_DIMENSION)return OUT;
    if(game->cell_tab[line][column] == KILLED)return OUT;
    if(game->cell_tab[line][column] == NORTH_KING || game->cell_tab[line][column] == SOUTH_KING)return BUSY;
    if (uses_range(game))
    {
        int player_line, player_column;

        find_player(game, current_player(game), &player_line, &player_column);
        copy_in_range(game, player_line, player_column, is_hex(game));

        if(range_tab[line][column] != CELL_AVAILABLE) return RULES;
    }
    
    game->cell_tab[line][column] = KILLED;
    
    if(game->current_player == NORTH){
        game->current_player = SOUTH;
    }
    else if(game->current_player == SOUTH){
        game->current_player = NORTH;
    }

    game->have_move = false;

    return OK;
}
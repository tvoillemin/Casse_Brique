v2 ball_p;
v2 ball_dp;
v2 ball_half_size;
b32 ball_base_speed;
f32 ball_speed_multiplier;

v2 player_p;
v2 player_dp;
v2 player_half_size;

struct {
	v2 p;
	v2 half_size;
	f32 ball_speed_multiplier;
	int life;
	u32 color;
} typedef Block;

Block blocks[256];
int next_block;

v2 arena_half_size;
b32 first_ball_movement;

b32 initialized = false;

#include "collision.c"

internal void
create_blocks(int num_x, int num_y, v2 block_half_size, f32 spacing_x, f32 spacing_y, f32 offset) {
	f32 x_offset = (f32)num_x * block_half_size.x * (1.f+spacing_x/2.f) - block_half_size.x;
	f32 y_offset = offset;
	for (int y = 0; y < num_y; ++y) {
		for (int x = 0; x < num_x; ++x) {
			Block *block = blocks+next_block++;
			if (next_block >= array_count(blocks)) {
				next_block = 0;
			}
			block->life = 1;
			block->half_size = block_half_size;
			block->p.x = x*block->half_size.x*(2.f+spacing_x) - x_offset;
			block->p.y = y*block->half_size.y*(2.f+spacing_y) - y_offset;
			block->color = make_color_from_grey(y*255/num_y);
			block->ball_speed_multiplier = 1+ (f32)y*1.25f/(f32)num_y;
		}
	}
}

enum {
	GM_NORMAL,
	GM_WALL,
	GM_CONSTRUCTION,
	GM_SPACED,
	/*GM_PONG,*/

	GM_COUNT,
} typedef Game_Mode;

Game_Mode current_game_mode;

internal void
restart_game(){

	if (current_game_mode >= GM_COUNT) current_game_mode = 0;
	else if (current_game_mode < 0) current_game_mode = GM_COUNT-1;
	ball_base_speed = 50;
	ball_dp.x = 0;
	ball_dp.y = -ball_base_speed;
	ball_p.x = 0;
	ball_p.y = 40;
	ball_half_size = (v2){.75, .75};
	ball_speed_multiplier = 1.f;

	player_p.y = -35;
	player_half_size = (v2){10, 2};

	arena_half_size = (v2){85, 45};
	first_ball_movement = true;

	next_block = 0;
	for (Block *block = blocks; block != blocks+array_count(blocks); block++) {
		block->life = 0;
	}

	switch(current_game_mode) {
		case GM_NORMAL: {
			create_blocks(20, 9, (v2){4, 2}, 0, 0, -4.f);
			
		} break;

		case GM_WALL: {
			create_blocks(19, 9, (v2){4, 2}, .2f, .2f, -4.f);
		} break;

		case GM_CONSTRUCTION: {
			create_blocks(21, 6, (v2){4, 2}, 0, 2.f, 0.f);
		} break;

		case GM_SPACED: {
			create_blocks(10, 6, (v2){4, 2}, 2.f, 2.f, 0.f);
		} break;

		invalid_default_case;
	}
}


internal void
simulate_game(Input *input, f32 dt) {

	if (!initialized) {
		initialized = true;
		Game_Mode current_game_mode = 0;
		restart_game();
	}

	v2 player_desired_p;
	player_desired_p.x = pixels_to_world(input->mouse).x;
	player_desired_p.y = player_p.y;
	v2 ball_desired_p = add_v2(ball_p, mul_v2(ball_dp, dt));

	if (ball_dp.y < 0 && is_colliding(player_p, player_half_size, ball_desired_p, ball_half_size)) {
		first_ball_movement = false;
		// Player collision with ball
		ball_dp.y *= -1;
		ball_dp.x = (ball_p.x - player_p.x)*7.5f;
		ball_dp.x += clamp(-25, player_dp.x*.5f, 25);
		ball_dp.x = (ball_p.x - player_p.x)*7.5f;
		ball_desired_p.y = player_p.y + player_half_size.y;
	}
	else if (ball_desired_p.x + ball_half_size.x > arena_half_size.x) {
		ball_desired_p.x = arena_half_size.x - ball_half_size.x;
		ball_dp.x *= -1;
	}
	else if (ball_desired_p.x - ball_half_size.x < -arena_half_size.x) {
		ball_desired_p.x = -arena_half_size.x + ball_half_size.x;
		ball_dp.x *= -1;
	}

	if (ball_desired_p.y + ball_half_size.y > arena_half_size.y) {
		ball_desired_p.y = arena_half_size.y - ball_half_size.y;
		ball_dp.y *= -1;
	}

	clear_screen_and_draw_rect((v2){0, 0}, arena_half_size, 0xff0000, 0x551100);

	for (Block *block = blocks; block != blocks+array_count(blocks); block++) {
		if (!block->life) continue;

		if (!first_ball_movement)
		// Ball/block collision
		{
			f32 diff = ball_desired_p.y - ball_p.y;
			if (diff != 0) {
				f32 collision_point;
				if (ball_dp.y > 0) collision_point = block->p.y - block->half_size.y - ball_half_size.y;
				else collision_point = block->p.y + block->half_size.y + ball_half_size.y;
				f32 t_y = (collision_point - ball_p.y) / diff;
				if (t_y >= 0.f && t_y <= 1.f) {
					f32 target_x = lerp(ball_p.x, t_y, ball_desired_p.x);
					if (target_x + ball_half_size.x > block->p.x - block->half_size.x &&
						target_x - ball_half_size.x < block->p.x + block->half_size.x) {
							ball_desired_p.y = lerp(ball_p.y, t_y, ball_desired_p.y);
							if (block->ball_speed_multiplier > ball_speed_multiplier) ball_speed_multiplier = block->ball_speed_multiplier;
							if (ball_dp.y > 0) ball_dp.y = -ball_base_speed * ball_speed_multiplier;
							else ball_dp.y = ball_base_speed * ball_speed_multiplier;
							block->life--;
					}
				}
			}

			diff = ball_desired_p.x - ball_p.x;
			if (diff != 0) {
				f32 collision_point;
				if (ball_dp.x > 0) collision_point = block->p.x - block->half_size.x - ball_half_size.x;
				else collision_point = block->p.x + block->half_size.x + ball_half_size.x;
				f32 t_x = (collision_point - ball_p.x) / diff;
				if (t_x >= 0.f && t_x <= 1.f) {
					f32 target_y = lerp(ball_p.y, t_x, ball_desired_p.y);
					if (target_y + ball_half_size.y > block->p.y - block->half_size.y &&
						target_y - ball_half_size.y < block->p.y + block->half_size.y) {
							ball_desired_p.x = lerp(ball_p.x, t_x, ball_desired_p.x);
							ball_dp.x *= -1;
							if (block->ball_speed_multiplier > ball_speed_multiplier) ball_speed_multiplier = block->ball_speed_multiplier;
							if (ball_dp.y > 0) ball_dp.y = ball_base_speed * ball_speed_multiplier;
							else ball_dp.y = -ball_base_speed * ball_speed_multiplier;
							block->life--;
					}
				}
			}
		}

		draw_rect(block->p, block->half_size, block->color);
	}

	ball_p = ball_desired_p;
	player_dp.x = (player_desired_p.x - player_p.x) / dt;
	player_p = player_desired_p;

	draw_rect(ball_p, ball_half_size, 0x00ffff);
	draw_rect(player_p, player_half_size, 0x00ff00);

		// Game over
	if (ball_desired_p.y - ball_half_size.y < -50) {
		restart_game(0);
	}

#if DEVELOPMENT
	if (pressed(BUTTON_LEFT)) restart_game(--current_game_mode);
	if (pressed(BUTTON_RIGHT)) restart_game(++current_game_mode);
#endif
}